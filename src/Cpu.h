#pragma once

#include "common.h"

#define SAFEZONE_V 1000
#define SAFEZONE_H 3000

class Cpu {

	friend void* computerThread(void * arg);

private:
	pthread_t thread;
	name_attach_t *attach = NULL;
	int fd = -1, n = 180;


public:
	Cpu();
	int join();

	void saveAirspace(const vector<PlaneInfo_t>& planes);

	void checkViolations(const vector<PlaneInfo_t>& planes);
	PlaneInfo_t calculatePosition(const PlaneInfo_t& plane, int t);
	bool notSafe(const PlaneInfo_t& thisPlane, const PlaneInfo_t& nextPlane);

	void sendPlaneToDisplay(PlaneInfo_t info, int i, int time);
	void alertDisplay(int id1, int id2, int t);
	void sendWindowToDisplay();

	void sendToComms(Msg msg);
	void sendToDisplay(Msg msg);
};
