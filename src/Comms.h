/*
	Comms.h
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Communications Thread class header.
 */
#pragma once

#include "common.h"

class Comms {

	// thread function as friend to share private attributes
	friend void* commsThread(void* arg);

private:
	pthread_t thread; // thread type
    name_attach_t *attach = NULL; // channel attach tyoe

public:
    Comms(); // constructor
    int join(); // join for main()

    void setup();	// setup IPC channel
    void destroy();	// destory IPC channel

    void send(Msg msg);	// send command to plane from CPU
    void sendToCpu(Msg msg); // send command to CPU from plane
};
