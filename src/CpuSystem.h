#ifndef CPUSYSTEM_H_
#define CPUSYSTEM_H_

#include <fcntl.h>
#include "common.h"
#include "Constants.h"

class CpuSystem {

	friend void* computerThread(void * arg);

private:
	pthread_t thread;
	name_attach_t *attach = NULL;
	int coid = 0;
	timer_t timerId;
	int time = 0;
	int fd;


public:
	CpuSystem();
	int join();

	vector<PlaneInfo_t> sendRadarCommand();
	void sendToDisplay(PlaneInfo_t info);
	void storeAirspace(const vector<PlaneInfo_t>& planes);

};


#endif
