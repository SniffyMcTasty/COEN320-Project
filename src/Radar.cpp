#include "Radar.h"

void* radarThread(void* arg) {

	Radar& radar = *((Radar*)arg);
	radar.setupChannel();

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

		case MsgType::RADAR:
			if (msg.hdr.subtype == MsgSubtype::REQ) {

				vector<Plane*> planes;
				planes = radar.primaryPulse();

				vector<PlaneInfo_t> planeStats;
				for (Plane* plane : planes)
					planeStats.push_back(radar.secondary(plane));

				int replyCnt = planeStats.size();
				MsgReply(rcvid, EOK, (void*) &replyCnt, sizeof(replyCnt));

				for (PlaneInfo_t p : planeStats) {
					Msg reply;
					reply.hdr.type = MsgType::RADAR;
					reply.hdr.subtype = MsgSubtype::REPLY;
					reply.info = p;
					MsgSend(radar.cpuThreadcoid, (void*)&reply, sizeof(reply), 0, 0);
				}

				break;
			}
			break;

		case MsgType::EXIT:
			cout << "Exit Radar Thread" << endl;
			exit = true;

		default:
			MsgReply(rcvid, EOK, 0, 0);
			break;
		}
	}

	radar.destroyChannel();
	pthread_exit(NULL);
}

Radar::Radar(vector<Plane*>* planes) {
	this->airspace = planes;
	if (pthread_create(&thread, NULL, radarThread, (void*)this))
		cout << "ERROR: MAKING RADAR THREAD" << endl;
}

int Radar::join() {
	return pthread_join(thread, NULL);
}

void Radar::setupChannel() {
	if ((attach = name_attach(NULL, RADAR_CHANNEL, 0)) == NULL) {
		cout << "ERROR: CREATING RADAR SERVER" << endl;
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

