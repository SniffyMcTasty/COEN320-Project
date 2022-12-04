#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <deque>
#include <pthread.h>
#include <signal.h>
#include <sys/dispatch.h>
#include <ncurses.h>
#include <algorithm>
#include <fcntl.h>
using namespace std;

extern pthread_mutex_t mtx;

struct PlaneInfo_t
{
    int id, x, y, z, dx, dy, dz, fl;
    friend ostream &operator<<(ostream &out, const PlaneInfo_t &info);
    string toString() const {
    	stringstream ss;
    	ss << "{";
        ss << "  ID=" << setw(4) << id;
        ss << "  X=" << setw(6) << to_string(x);
        ss << "  Y=" << setw(6) << to_string(y);
        ss << "  Z=" << setw(5) << to_string(z);
        ss << "  dX=" << setw(5) << to_string(dx);
        ss << "  dY=" << setw(5) << to_string(dy);
        ss << "  dZ=" << setw(3) << to_string(dz);
        ss << "  FL=" << setw(3) << to_string(fl);
        ss << "  }";
        return ss.str();
    }
};

// 4 6 6 5 4 4 4 3

inline ostream &operator<<(ostream &out, const PlaneInfo_t &info) {
    out << info.toString();
    return out;
}

////////////////////////////////////////////////// IPC

typedef struct _pulse msg_header_t;

struct Msg {
    msg_header_t hdr;
    PlaneInfo_t info;
    union {
    	float floatValue;
    	int intValue;
    };
};

enum MsgType : _Uint16t { TIMEOUT, RADAR, COMMAND, ALERT, EXIT }; // add msgs here
enum MsgSubtype : _Uint16t { CHANGE_SPEED, CHANGE_ALTITUDE, CHANGE_POSITION, CHANGE_WINDOW, REQ, REPLY }; // add subtypes here

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

#define ALERT_GAP 32

////////////////////////////////////////////////// UTILITY

inline int randRange(int lo, int hi)
{
    return rand() % (hi - lo) + lo;
}
