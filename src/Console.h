#pragma once

#include "common.h"
#include "Constants.h"

class Console {

	friend void* consoleThread(void* arg);

private:
	pthread_t thread;
	int coid;

	bool exit = false;

public:
	Console();
	int join();
};
