#include "Plane.h"

void *planeThread(void *arg)
{

    Plane &plane = *((Plane *)arg);
    plane.setup();

    cout << "Created: " << plane << endl;

    while (plane.inZone())
    {
        int rcvid;
        Msg msg;

        if ((rcvid = MsgReceive(plane.attach->chid, &msg, sizeof(msg), NULL)) == -1)
            break;

        if (!rcvid && (msg.hdr.code == _PULSE_CODE_DISCONNECT))
        {
            ConnectDetach(msg.hdr.scoid);
            continue;
        }

        if (msg.hdr.type == _IO_CONNECT)
        {
            MsgReply(rcvid, EOK, NULL, 0);
            continue;
        }

        if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX)
        {
            MsgError(rcvid, ENOSYS);
            continue;
        }

        switch (msg.hdr.type)
        {

        case MsgType::TIMEOUT:
            plane.updatePosition();
            cout << "Update:  " << plane << endl;
            MsgReply(rcvid, EOK, 0, 0);
            break;

        case MsgType::REQ:
            if (msg.hdr.subtype == MsgSubType ::RADAR)
            {

            	cout << " * RADAR REQEUST: " << plane.info << endl;
            	 MsgReply(rcvid, EOK, 0, 0);
            	 plane.radarReply(msg.hdr.scoid);
            	 break;
            }

            break;

        default:
            break;
        }


    }

    plane.destroy();
    cout << "Plane " << plane.info.id << " exited airspace" << endl;
    pthread_exit(NULL);
}

ostream &operator<<(ostream &out, const Plane &plane)
{
    out << plane.info;
    return out;
}

Plane::Plane(PlaneInfo info) : info{info}
{
    channel = to_string(info.x + info.y + info.z);
    pthread_create(&thread, NULL, planeThread, (void *)this);
}

int Plane::join()
{
    return pthread_join(thread, NULL);
}

void Plane::setup()
{
    setupChannel();
    setupTimer();
}

void Plane::setupChannel()
{
    if ((attach = name_attach(NULL, channel.c_str(), 0)) == NULL)
        pthread_exit(NULL);
    if ((coid = name_open(channel.c_str(), 0)) == -1)
        pthread_exit(NULL);
}

void Plane::setupTimer()
{
    sigevent event;
    event.sigev_notify = SIGEV_PULSE;
    event.sigev_coid = coid;
    event.sigev_code = MsgType::TIMEOUT;

    timer_create(CLOCK_MONOTONIC, &event, &timerId);

    itimerspec timerSpec{1, 0, 1, 0};
    timer_settime(timerId, 0, &timerSpec, NULL);
}

void Plane::destroy()
{
    destroyTimer();
    destroyChannel();
}

void Plane::destroyTimer()
{
    itimerspec off{0, 0, 0, 0};
    timer_settime(timerId, 0, &off, NULL);
    timer_delete(timerId);
}

void Plane::destroyChannel()
{
    name_close(coid);
    name_detach(attach, 0);
}

void Plane::updatePosition()
{
    info.x += info.dx;
    info.y += info.dy;
    info.z += info.dz;
    info.fl = info.z / 100;
}

bool Plane::inZone()
{
    return !(info.x < LOWER_X || info.x > UPPER_X || info.y < LOWER_Y || info.y > UPPER_Y || info.z < LOWER_Z || info.z > UPPER_Z);
}

int Plane::ping()
{
    Msg msg;
    msg.hdr.type = MsgType::REQ;
    msg.hdr.subtype = MsgSubType::RADAR;

    memset(&msg.info, 0, sizeof(PlaneInfo));
    return MsgSend(coid, &msg, sizeof(msg), NULL, 0);
}
int Plane:: radarReply(_Int32t scoid){
	Msg msg;
	msg.hdr.type =MsgType::REPLY;
	msg.hdr.subtype =MsgSubType::RADAR;
	msg.info = info;
	 return MsgSend(scoid, &msg, sizeof(msg), NULL, 0);
}

PlaneInfo Plane::randomInfo()
{
    //	- Aircraft enters airspace flying in horizontal plane (x or y plane) at
    //	constant velocity.
    //	- Maintains speed and altitude unless commanded to change.
    int id, x, y, z, dx, dy, dz, fl;
    id = randRange(100, 999);
    if (rand() % 2)
    {
        x = randRange(LOWER_X + LOWER_X / 4, UPPER_X - UPPER_X / 4);
        y = rand() % 2 ? LOWER_Y : UPPER_Y;
        dx = 10 * randRange(-100, 100);
        dy = 10 * randRange(80, 120) * (y == LOWER_Y ? 1 : -1);
    }
    else
    {
        x = rand() % 2 ? LOWER_X : UPPER_X;
        y = randRange(LOWER_Y + LOWER_Y / 4, UPPER_Y - UPPER_Y / 4);
        dx = 10 * randRange(80, 120) * (x == LOWER_X ? 1 : -1);
        dy = 10 * randRange(-100, 100);
    }
    // z = randRange(LOWER_Z + LOWER_Z / 4, UPPER_Z - UPPER_Z / 4);
    // dz = 0;
    z = LOWER_Z;
    dz = (AIRSPACE_Z / 10);
    fl = z / 100;
    return PlaneInfo{id, x, y, z, dx, dy, dz, fl};
}
