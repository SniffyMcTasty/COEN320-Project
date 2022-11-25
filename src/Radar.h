#pragma once

#include "common.h"
#include "Plane.h"

class Radar {

    friend void* radarThread(void* arg);

private:
    vector<Plane*>* airspace;
	pthread_t thread;
	string channel;
	name_attach_t *attach = NULL;
	int coid = 0;

	void setupChannel();
	void destroyChannel();

public:
	Radar(vector<Plane*>* planes);
	int join();
	vector<Plane*> primary();
	PlaneInfo secondary(Plane* plane);

	void getPlanes();
	void exit();

	bool noPlanes = true;
};
