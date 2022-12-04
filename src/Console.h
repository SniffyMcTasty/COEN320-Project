#pragma once

#include "common.h"
#include "Plane.h"

class Console {

	friend void* consoleThread(void* arg);

private:
	pthread_t thread;
	int coid;

	bool exit = false;
	bool& mainExit;

public:
	Console(bool& mainExit);
	int join();

	void parseWindowCmd(string& buffer);
	void parseSpeedCmd(string& buffer);
	void parseAltCmd(string& buffer);
	void parsePosCmd(string& buffer);

	void changeWindow(int n);
	void changeSpeed(int id, float percentChange);
	void changeAlt(int id, int alt);
	void changePos(int id, float angle);

	void saveCmd(int fd, const string& buffer);
	void sendExit(const char* channel);
};
