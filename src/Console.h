/*
	Console.h
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Console System Thread class header.
 */
#pragma once

#include "common.h"
#include "Plane.h"

class Console {

	// thread function as friend to share private attributes
	friend void* consoleThread(void* arg);

private:
	pthread_t thread;	// thread type
	bool exit = false;	// exit flag

public:
	Console();	// constructor
	int join();	// join for main()

	// functions that parse inputs from user
	void parseWindowCmd(string& buffer);
	void parseSpeedCmd(string& buffer);
	void parseAltCmd(string& buffer);
	void parsePosCmd(string& buffer);
	void parseInfoCmd(string& buffer);

	// functions that sends correct inputs to CPU thread
	void changeWindow(int n);
	void changeSpeed(int id, double v);
	void changeAlt(int id, int alt);
	void changePos(int id, float angle);
	void dispInfo(int id);

	// stores a command into airspace
	void saveCmd(int fd, const string& buffer);

	// exit threads that are blocked with MsgReceive()
	void sendExit(const char* channel);
};
