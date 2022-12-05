#pragma once

#include "common.h"
#include "Plane.h"

class Console {

	friend void* consoleThread(void* arg);

private:
	pthread_t thread;
	int coid;

	bool exit = false;

public:
	Console();
	int join();

	void parseWindowCmd(string& buffer);
	void parseSpeedCmd(string& buffer);
	void parseAltCmd(string& buffer);
	void parsePosCmd(string& buffer);
	void parseInfoCmd(string& buffer);

	void changeWindow(int n);
	void changeSpeed(int id, double v);
	void changeAlt(int id, int alt);
	void changePos(int id, float angle);

	void dispInfo(int id);

	void saveCmd(int fd, const string& buffer);
	void sendExit(const char* channel);
};
