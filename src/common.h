#pragma once

#include <iostream>
// #include <vector>
#include <pthread.h>
#include <signal.h>
#include <sys/dispatch.h>
using namespace std;

struct PlaneInfo_t
{
    int id, x, y, z, dx, dy, dz, fl;
    friend ostream &operator<<(ostream &out, const PlaneInfo_t &info);
};

inline ostream &operator<<(ostream &out, const PlaneInfo_t &info)
{
    out << "{";
    out << "  ID=" << info.id;
    out << "  X=" << info.x;
    out << "  Y=" << info.y;
    out << "  Z=" << info.z;
    out << "  dX=" << info.dx;
    out << "  dY=" << info.dy;
    out << "  dZ=" << info.dz;
    out << "  FL=" << info.fl;
    out << "  }";
    return out;
}

////////////////////////////////////////////////// IPC

typedef struct _pulse msg_header_t;

struct Msg
{
    msg_header_t hdr;
    PlaneInfo_t info;
};

enum MsgType : _Uint16t
{
    TIMEOUT,
    REQ,
    REPLY
};
enum MsgSubType : _Uint16t
{
    RADAR

};

////////////////////////////////////////////////// MACROS

#define AIRSPACE_X 100000
#define AIRSPACE_Y 100000
#define AIRSPACE_Z 25000

#define LOWER_X 0
#define LOWER_Y 0
#define LOWER_Z 15000

#define UPPER_X (LOWER_X + AIRSPACE_X)
#define UPPER_Y (LOWER_Y + AIRSPACE_Y)
#define UPPER_Z (LOWER_Z + AIRSPACE_Z)

////////////////////////////////////////////////// UTILITY

inline int randRange(int lo, int hi)
{
    return rand() % (hi - lo) + lo;
}
