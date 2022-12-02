#include "CpuSystem.h"

void* computerThread(void* arg) {

	CpuSystem& cpu = *((CpuSystem*)arg);

	// create channel to listen on for this thread
	cpu.attach = name_attach(NULL, CPU_CHANNEL, 0);
	if (cpu.attach == NULL) {
		cout << "ERROR: CREATING CPU SERVER" << endl;
		pthread_exit(NULL);
	}

	// open channel to talk to this threads server (loop-back)
	cpu.coid = name_open(CPU_CHANNEL, 0);
	if (cpu.coid == -1) {
		cout << "ERROR: CREATING CPU CLIENT" << endl;
		pthread_exit(NULL);
	}

	// timer setup
	sigevent event;
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = cpu.coid;
	event.sigev_code = MsgType::TIMEOUT;

	if (timer_create(CLOCK_MONOTONIC, &event, &cpu.timerId) < 0) {
		cout << "ERROR: CREATING PLANE TIMER" << endl;
		pthread_exit(NULL);
	}

	itimerspec timerSpec{ 5, 0, 5, 0 };



	name_detach(cpu.attach, 0);
	name_close(cpu.coid);
	pthread_exit(NULL);
}

CpuSystem::CpuSystem() {
	if (pthread_create(&thread, NULL, computerThread, (void*)this))
		cout << "ERROR: MAKING CPU THREAD" << endl;
}

int CpuSystem::join() {
	return pthread_join(thread, NULL);
}
