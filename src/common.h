#pragma once

#include <iostream>
#include <vector>
#include <pthread.h>
#include <signal.h>
#include <sys/dispatch.h>
using namespace std;

struct PlaneInfo {
    int id, x, y, z, dx, dy, dz, fl;
    friend ostream &operator<<(ostream &out, const PlaneInfo &info);
    string toString() const {
    	string s = "{";
        s += "  ID=" + to_string(id);
        s += "  X=" + to_string(x);
        s += "  Y=" + to_string(y);
        s += "  Z=" + to_string(z);
        s += "  dX=" + to_string(dx);
        s += "  dY=" + to_string(dy);
        s += "  dZ=" + to_string(dz);
        s += "  FL=" + to_string(fl);
        s += "  }";
        return s;
    }
};

inline ostream &operator<<(ostream &out, const PlaneInfo &info) {
    out << info.toString();
    return out;
}

////////////////////////////////////////////////// IPC

typedef struct _pulse msg_header_t;

struct Msg {
    msg_header_t hdr;
    PlaneInfo info;
};

enum MsgType : _Uint16t { TIMEOUT, RADAR, EXIT };
enum MsgSubType : _Uint16t { REQ, REPLY };

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

inline int randRange(int lo, int hi) {
    return rand() % (hi - lo) + lo;
}
