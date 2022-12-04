#pragma once

#include "common.h"

class Plane
{
    friend ostream &operator<<(ostream &out, const Plane &plane);
    friend void *planeThread(void *arg);

private:
    PlaneInfo_t info;
    pthread_t thread;
    name_attach_t *attach = NULL;
    int coid = 0;
    timer_t timerId = 0;
    bool changeAltFlag = false;
    int finalAlt = 0;

    int time = 0;

	void setParameters();
    void setup();
    void setupChannel();
    void setupTimer();
    void destroy();
    void destroyTimer();
    void destroyChannel();
    void updatePosition();

public:
    Plane(PlaneInfo_t info);
    Plane(int time, int id);
    int join();
    bool inZone();
	string format();
    string toString() const;
    PlaneInfo_t ping();

    const char* getChannel() {
    	return to_string(info.id).c_str();
    }

    static PlaneInfo_t randomInfo();
};

// make airspace reachable by CONSOLE task when exit command sent
extern vector<Plane*> airspace;
