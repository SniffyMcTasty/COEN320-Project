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

    void setup();
    void setupChannel();
    void setupTimer();
    void destroy();
    void destroyTimer();
    void destroyChannel();
    void updatePosition();

public:
    string channel;
    Plane(PlaneInfo_t info);
    int join();
    bool inZone();
    string toString() const;
    PlaneInfo_t ping();

    static PlaneInfo_t randomInfo();
};
