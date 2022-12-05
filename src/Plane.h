/*
	Plane.h
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Plane Thread class header.
*/
#pragma once

#include "common.h"

class Plane
{
	// stream insertion (output) operator
    friend ostream &operator<<(ostream &out, const Plane &plane);

	// thread function as friend to share private attributes
    friend void *planeThread(void *arg);

private:
    PlaneInfo_t info;	// plane's info
    pthread_t thread;	// thread type
    name_attach_t *attach = NULL;	// channel attach type
    int coid = 0;	// loopback connection ID
    timer_t timerId = 0;	// timer id
    bool changeAltFlag = false;	// flag for altitude change in progress
    int finalAlt = 0;	// final desired altitude for change

    double v = 0;	// velocity magnitude for some commands
    int time = 0;	// timer during input generation for plane release

    // class/setup-related functions
	void setParameters();
    void setup();
    void setupChannel();
    void setupTimer();
    void destroy();
    void destroyTimer();
    void destroyChannel();
    void updatePosition();

public:
    Plane(PlaneInfo_t info); // constructor with PlaneInfo_t
    Plane(int time, int id); // constructor with time and id for input generation
    int join();

    bool inZone();	// check if plane is in airspace

    string format(); // convert to string to write to input file
    string toString() const; // convert to string
    PlaneInfo_t ping();	// radar ping, send back position

    // getter for channel
    const char* getChannel() { return to_string(info.id).c_str(); }

    // static random PlaneInfo_t return
    static PlaneInfo_t randomInfo();
};
