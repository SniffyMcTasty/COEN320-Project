/*
 * Plane.h
 *

 */

#ifndef PLANE_H_
#define PLANE_H_

#include "common.h"
class Plane
{
    friend ostream &operator<<(ostream &out, const Plane &plane);
    friend void *planeThread(void *arg);

private:
    PlaneInfo_t info;
    pthread_t thread;
    string channel;
    name_attach_t *attach;
    int coid;
    timer_t timerId;

    void setup();
    void setupChannel();
    void setupTimer();
    void destroy();
    void destroyTimer();
    void destroyChannel();
    void updatePosition();

public:
    Plane(PlaneInfo_t info);
    int join();
    bool inZone();
    int ping();
    int radarReply(_Int32t scoid);

    static PlaneInfo_t randomInfo();
};





#endif /* PLANE_H_ */
