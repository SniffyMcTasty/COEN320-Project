#include "Radar.h"

void* radarThread(void* arg) {

	Radar& radar = *((Radar*)arg);
	radar.setupChannel();

	// timer setup
	sigevent event;
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = radar.coid;
	event.sigev_code = MsgType::TIMEOUT;

	if (timer_create(CLOCK_MONOTONIC, &event, &radar.timerId) < 0) {
		cout << "ERROR: CREATING PLANE TIMER" << endl;
		pthread_exit(NULL);
	}

	itimerspec timerSpec{ 5, 0, 5, 0 };
	if (timer_settime(radar.timerId, 0, &timerSpec, NULL) < 0) {
		cout << "ERROR: STARTING CPU TIMER" << endl;
		pthread_exit(NULL);
	}

	cout << "Running Radar Thread" << endl;

	bool exit = false;
	while (!exit) {
		Msg msg;

		int rcvid = MsgReceive(radar.attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == -1) break;
		if (!rcvid && (msg.hdr.code == _PULSE_CODE_DISCONNECT))	{ ConnectDetach(msg.hdr.scoid);		continue; }
		if (msg.hdr.type == _IO_CONNECT) 						{ MsgReply(rcvid, EOK, NULL, 0);	continue; }
		if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX)	{ MsgError(rcvid, ENOSYS);			continue; }

		switch (msg.hdr.type) {

		case MsgType::TIMEOUT:
		{
			radar.time += 5;
			vector<Plane*> planes;
			planes = radar.primaryPulse();

			vector<PlaneInfo_t> planeStats;
			for (Plane* plane : planes)
				planeStats.push_back(radar.secondary(plane));

			msg.hdr.type = RADAR;
			msg.hdr.subtype = planeStats.size();
			msg.intValue = radar.time;
			memset(&msg.info, 0, sizeof(msg.info));
			MsgSend(radar.cpuThreadcoid, &msg, sizeof(msg), 0, 0);

			for (PlaneInfo_t p : planeStats) {
				msg.info = p;
				MsgSend(radar.cpuThreadcoid, &msg, sizeof(msg), 0, 0);
			}
		}
			break;

		case MsgType::EXIT:
			exit = true;
			MsgReply(rcvid, EOK, 0, 0);
			break;

		default:
			MsgReply(rcvid, EOK, 0, 0);
			break;
		}
	}

	itimerspec off{ 0, 0, 0, 0 };
	timer_settime(radar.timerId, 0, &off, NULL);
	timer_delete(radar.timerId);
	radar.destroyChannel();
	pthread_exit(NULL);
}

Radar::Radar(vector<Plane*>* planes) {
	this->airspace = planes;
	if (pthread_create(&thread, NULL, radarThread, (void*)this))
		cout << "ERROR: MAKING RADAR THREAD" << endl;
}

int Radar::join() {
	cout << "Joining Radar Thread" << endl;
	return pthread_join(thread, NULL);
}

void Radar::setupChannel() {
	if ((attach = name_attach(NULL, RADAR_CHANNEL, 0)) == NULL) {
		cout << "ERROR: CREATING RADAR SERVER" << endl;
		pthread_exit(NULL);
	}

	if ((coid = name_open(RADAR_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING RADAR LOOPBACK" << endl;
		pthread_exit(NULL);
	}

	if ((cpuThreadcoid = name_open(CPU_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO CPU" << endl;
		pthread_exit(NULL);
	}
}

void Radar::destroyChannel() {
//	name_close(coid);
	name_detach(attach, 0);
}

vector<Plane*> Radar::primaryPulse() {
	vector<Plane*> planesInZone;
	for (Plane* p : *airspace)
		if (p->inZone())
			planesInZone.push_back(p);
	return planesInZone;
}

PlaneInfo_t Radar::secondary(Plane* plane) {
	return plane->ping();
}

