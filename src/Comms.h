#pragma once

#include "common.h"

class Comms {

	friend void* commsThread(void* arg);

private:
	pthread_t thread;
    name_attach_t *attach = NULL;


public:
    Comms();
    int join();

    void send(Msg msg);
};
