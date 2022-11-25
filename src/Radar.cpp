#include "Radar.h"

void* radarThread(void* arg) {

	Radar& radar = *((Radar*)arg);
	radar.setupChannel();

	cout << "Radar Thread Running" << endl;

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
			if (msg.hdr.subtype == MsgSubType::REQ) {

				vector<PlaneInfo> planeStats;
				vector<Plane*> planes;

				planes = radar.primary();

				radar.noPlanes = planes.empty();
				if (radar.noPlanes)
					cout << "No Planes" << endl;

				for (Plane* plane : planes) {
					PlaneInfo info = radar.secondary(plane);
					cout << info << endl;
					planeStats.push_back(info);
				}
				MsgReply(rcvid, EOK, 0, 0);
				break;
			}
			break;

		case MsgType::EXIT:
			cout << "Exiting Radar Thread" << endl;
			exit = true;

		default:
			MsgReply(rcvid, EOK, 0, 0);
			break;
		}

	}

	radar.destroyChannel();
	cout << "Radar Thread Exiting" << endl;
	pthread_exit(NULL);
}

Radar::Radar(vector<Plane*>* planes) {
	this->airspace = planes;
	channel = "radar";
	if (pthread_create(&thread, NULL, radarThread, (void*)this))
		cout << "ERROR: MAKING RADAR THREAD" << endl;
}

int Radar::join() {
	return pthread_join(thread, NULL);
}

void Radar::setupChannel() {
	if ((attach = name_attach(NULL, channel.c_str(), 0)) == NULL) {
		cout << "ERROR: CREATING RADAR SERVER" << endl;
		pthread_exit(NULL);
	}

	if ((coid = name_open(channel.c_str(), 0)) == -1) {
		cout << "ERROR: CREATING RADAR CLIENT" << endl;
		pthread_exit(NULL);
	}
}

void Radar::destroyChannel() {
	name_close(coid);
	name_detach(attach, 0);
}

vector<Plane*> Radar::primary() {
	vector<Plane*> planesInZone;
	for (Plane* p : *airspace)
		if (p->inZone())
			planesInZone.push_back(p);
	return planesInZone;
}

PlaneInfo Radar::secondary(Plane* plane) {
	return plane->ping();
}

void Radar::getPlanes() {
	Msg msg;
	msg.hdr.type = MsgType::RADAR;
	msg.hdr.subtype = MsgSubType::REQ;
	if (MsgSend(coid, &msg, sizeof(msg), 0, 0) < 0)
		cout << "ERROR: GETPLANES FAILED" << endl;
}

void Radar::exit() {
	Msg msg;
	msg.hdr.type = MsgType::EXIT;
	MsgSend(coid, &msg, sizeof(msg), 0, 0);
}
