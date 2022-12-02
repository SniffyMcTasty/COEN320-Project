#ifndef CPUSYSTEM_H_
#define CPUSYSTEM_H_

#include "common.h"

#define CPU_CHANNEL "CPU_CHANNEL"

class CpuSystem {

	friend void* computerThread(void * arg);

private:
	pthread_t thread;
	name_attach_t *attach = NULL;
	int coid = 0;
	timer_t timerId;


public:
	CpuSystem();
	int join();

};


#endif
