#pragma once

#include "common.h"
#include "Constants.h"
#include "Plane.h"

#define RADAR_CHANNEL "radarChannel"

class Radar {

    friend void* radarThread(void* arg);

private:
    vector<Plane*>* airspace;
	pthread_t thread;
//	string channel;
	name_attach_t *attach = NULL;
	int coid = 0;
//	bool _exit = false;

	void setupChannel();
	void destroyChannel();

public:
	Radar(vector<Plane*>* planes);
	int join();
	vector<Plane*> primary();
	PlaneInfo_t secondary(Plane* plane);

	void getPlanes();
//	void exit();

	bool setup = false;
};
