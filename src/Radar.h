#pragma once

#include "common.h"
#include "Plane.h"



class Radar {

    friend void* radarThread(void* arg);

private:
    vector<Plane*>* airspace;
	pthread_t thread;
	int coid = 0;
	timer_t timerId;
	int time = 0;
//	string channel;
	name_attach_t *attach = NULL;
	int cpuThreadcoid = 0;
//	bool _exit = false;

	void setupChannel();
	void destroyChannel();

public:
	Radar(vector<Plane*>* planes);
	int join();
	vector<Plane*> primaryPulse();
	PlaneInfo_t secondary(Plane* plane);

	void getPlanes();
//	void exit();

//	bool setup = false;
};
