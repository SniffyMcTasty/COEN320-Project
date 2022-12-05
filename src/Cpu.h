/*
	Cpu.h
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Computer System Thread class header.
 */
#pragma once

#include "common.h"

// minimum inter-plane safezone (horizontal and vertical)
#define SAFEZONE_HORIZONTAL	3000
#define SAFEZONE_VERTICAL	1000

class Cpu {

	// thread function as friend to share private attributes
	friend void* computerThread(void * arg);

private:
	pthread_t thread;	// thread type
	name_attach_t *attach = NULL;	// channel attach type
	int fd = -1, n = 180;	// file descriptor and window 'n'


public:
	Cpu();		// constructor
	int join();	// join for main()

	void setup();	// setup thread resources (IPC, file)
	void destroy();	// close thread resource (IPC, file)

	// store current airspace planes in file
	void saveAirspace(const vector<PlaneInfo_t>& planes, int time);

	// functions to check if minimum distances respected
	void checkViolations(const vector<PlaneInfo_t>& planes);
	PlaneInfo_t calculatePosition(const PlaneInfo_t& plane, int t);
	bool notSafe(const PlaneInfo_t& thisPlane, const PlaneInfo_t& nextPlane);

	// functions to send data to display systems
	void sendPlaneToDisplay(PlaneInfo_t info, int i, int time);
	void alertDisplay(int id1, int id2, int t);
	void sendWindowToDisplay();

	// sends plane info commands back/forth
	void sendToComms(Msg msg);
	void sendToDisplay(Msg msg);
};
