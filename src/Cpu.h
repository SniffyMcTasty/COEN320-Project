#ifndef CPU_H_
#define CPU_H_


#include "common.h"

#define SAFEZONE_V 1000
#define SAFEZONE_H 3000

class Cpu {

	friend void* computerThread(void * arg);

private:
	pthread_t thread;
	name_attach_t *attach = NULL;
	timer_t timerId;
	int coid = 0;
	int time = 0;
	int fd;
	int n = 180;


public:
	Cpu();
	int join();

	void saveAirspace(const vector<PlaneInfo_t>& planes);

	void checkViolations(const vector<PlaneInfo_t>& planes);
	PlaneInfo_t calculatePosition(const PlaneInfo_t& plane, int t);
	bool notSafe(const PlaneInfo_t& thisPlane, const PlaneInfo_t& nextPlane);

	vector<PlaneInfo_t> sendRadarCommand();

	void sendPlaneToDisplay(PlaneInfo_t info, int i, int time);
	void alertDisplay(int id1, int id2, int t);

	void sendWindowToDisplay();

};


#endif
