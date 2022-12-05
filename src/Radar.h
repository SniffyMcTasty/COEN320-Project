/*
	Radar.h
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Radar System Thread class header.
*/
#pragma once

#include "common.h"
#include "Plane.h"

class Radar {

	// thread function as friend to share private attributes
    friend void* radarThread(void* arg);

private:
    vector<Plane*>* airspace;	// pointer to planes in air
	pthread_t thread;			// thread type
	int coid = 0;				// connection ID for loopback
	timer_t timerId;			// timer id
	int time = 0;				// current time
	name_attach_t *attach = NULL; // channel attach type
	int cpuThreadcoid = 0;	// connection id to CPU

	void setup();
	void destroy();

public:
	Radar(vector<Plane*>* planes); // constructor
	int join();

	// radar functions
	vector<Plane*> primaryPulse();
	PlaneInfo_t secondary(Plane* plane);
};
