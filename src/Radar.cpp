/*
	Radar.cpp
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Radar System Thread class.
		Sends pulse to find planes every 5 secs and send to CPU system.
		Uses planes* vector to find planes (Primary Radar) then pings each plane
		individually to get position (secondary radar)
*/
#include "Radar.h"

// Thread function. Uses void pointer arg to pass pointer to object calling the function
void* radarThread(void* arg) {

	Radar& radar = *((Radar*)arg); // store Plane object that started thread
	radar.setup(); 	// setup timer and IPCs

	cout << "Running Radar Thread" << endl;

	bool exit = false;  // exit flag
	while (!exit) {
		Msg msg;
		int rcvid = MsgReceive(radar.attach->chid, &msg, sizeof(msg), NULL);  // block and wait for msg

		// message receive error
		if (rcvid == -1) break;
		// client wants to disconnect
		if (!rcvid && (msg.hdr.code == _PULSE_CODE_DISCONNECT))	{ ConnectDetach(msg.hdr.scoid);		continue; }
		// new connection
		if (msg.hdr.type == _IO_CONNECT) 						{ MsgReply(rcvid, EOK, NULL, 0);	continue; }
		// other error
		if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX)	{ MsgError(rcvid, ENOSYS);			continue; }

		// expected message types
		switch (msg.hdr.type) {

		// get planes on timeout and send to CPU
		case MsgType::TIMEOUT:
		{
			// increment timestamp
			radar.time += 5;
			vector<Plane*> planes;
			planes = radar.primaryPulse();

			vector<PlaneInfo_t> planeStats;
			for (Plane* plane : planes)
				planeStats.push_back(radar.secondary(plane));

			// make common message type
			msg.hdr.type = RADAR;
			msg.hdr.subtype = planeStats.size();
			msg.intValue = radar.time;
			memset(&msg.info, 0, sizeof(msg.info));

			// send time and total planes first,
			MsgSend(radar.cpuThreadcoid, &msg, sizeof(msg), 0, 0);
			// then send all the planes
			for (PlaneInfo_t p : planeStats) {
				msg.info = p;
				MsgSend(radar.cpuThreadcoid, &msg, sizeof(msg), 0, 0);
			}
		}
			break;

		// exit message
		case MsgType::EXIT:
			exit = true;
			MsgReply(rcvid, EOK, 0, 0);
			break;

		default:
			MsgReply(rcvid, EOK, 0, 0);
			break;
		}
	}

	radar.destroy();
	pthread_exit(NULL);
}

// constructor with Plane* airspace
Radar::Radar(vector<Plane*>* planes) {
	this->airspace = planes;
	if (pthread_create(&thread, NULL, radarThread, (void*)this))
		cout << "ERROR: MAKING RADAR THREAD" << endl;
}

// make join available to main()
int Radar::join() {
	cout << "Joining Radar Thread" << endl;
	return pthread_join(thread, NULL);
}

// setup channels and timer
void Radar::setup() {
	// channels setup
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

	// timer signal setup
	sigevent event;
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = coid;
	event.sigev_code = MsgType::TIMEOUT;

	// create timer
	if (timer_create(CLOCK_MONOTONIC, &event, &timerId) < 0) {
		cout << "ERROR: CREATING PLANE TIMER" << endl;
		pthread_exit(NULL);
	}

	// timer specs and start
	itimerspec timerSpec{ 5, 0, 5, 0 };
	if (timer_settime(timerId, 0, &timerSpec, NULL) < 0) {
		cout << "ERROR: STARTING CPU TIMER" << endl;
		pthread_exit(NULL);
	}
}

// destroy channels and timer
void Radar::destroy() {
	itimerspec off{ 0, 0, 0, 0 };
	timer_settime(timerId, 0, &off, NULL);
	timer_delete(timerId);

	name_close(coid);
	name_close(cpuThreadcoid);
	name_detach(attach, 0);
}

// send the primary radar pulse to see how many planes are in airspace
vector<Plane*> Radar::primaryPulse() {
	vector<Plane*> planesInZone;
	for (Plane* p : *airspace)
		if (p->inZone())
			planesInZone.push_back(p);
	return planesInZone;
}

// send the secondary radar pulse for more details
PlaneInfo_t Radar::secondary(Plane* plane) {
	return plane->ping();
}

