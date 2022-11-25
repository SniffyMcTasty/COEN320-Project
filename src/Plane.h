#pragma once

#include "common.h"

class Plane
{
    friend ostream &operator<<(ostream &out, const Plane &plane);
    friend void *planeThread(void *arg);

private:
    PlaneInfo info;
    pthread_t thread;
    string channel;
    name_attach_t *attach = NULL;
    int coid = 0;
    timer_t timerId = 0;

    void setup();
    void setupChannel();
    void setupTimer();
    void destroy();
    void destroyTimer();
    void destroyChannel();
    void updatePosition();

public:
    Plane(PlaneInfo info);
    int join();
    bool inZone();
    string toString() const;
    PlaneInfo ping();

    static PlaneInfo randomInfo();
};
