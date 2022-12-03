#ifndef CPUSYSTEM_H_
#define CPUSYSTEM_H_

#include "common.h"
#include "Constants.h"

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

	vector<PlaneInfo_t> sendRadarCommand();
};


#endif
