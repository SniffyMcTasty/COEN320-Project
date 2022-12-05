/*
	common.h
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Contains common libraries, macros, and functions shared by classes.
 */
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
#include <cmath>
using namespace std;

#define PI 3.1415926536	// PI value
#define ALERT_GAP 33	// width in CHARs of the alert column

/************************* PLANE CREATION *************************/

#define AIRSPACE_WIDTH	100000	// x and y dimensions of airspace
#define AIRSPACE_HEIGHT	25000	// z dimension of airspace

#define MIN_Z 15000						// airspace altitude lower bound
#define MAX_Z (MIN_Z + AIRSPACE_HEIGHT)	// airspace altitude upper bound

#define MIN_SPEED 		733 	// typical min speed for aircrafts is 500 mph (~733 ft/s)
#define MAX_SPEED 		1026	// typical max speed for aircrafts is 700 mph (~1026 ft/s)
#define SPEED_INTERVAL	293		// difference between min and max speed

#define ID_MIN		1000	// unique ID start
#define ID_INTERVAL	9000	// unique ID end

/************************* FILE NAMES ******************************/

#define INPUT_FILENAME		"/data/home/qnxuser/loadInput.txt"
#define HISTORY_FILENAME	"/data/home/qnxuser/history.txt"
#define COMMANDS_FILENAME	"/data/home/qnxuser/commands.txt"

/************************* IPC CHANNEL NAMES *************************/

#define RADAR_CHANNEL	"radarChannel"
#define CPU_CHANNEL		"cpuChannel"
#define DISPLAY_CHANNEL	"displayChannel"
#define COMMS_CHANNEL	"commsChannel"

/****************************** TYPES ******************************/

// contains all the Plane's info
struct PlaneInfo_t {
	// ID, position (x, y, z), speed (dx, dy, dz), flight level
    int id, x, y, z, dx, dy, dz, fl;

    // convert to string
    string toString() const {
    	stringstream ss;	// use string stream to make things easier
        ss << setw(4) << id;
        ss << "  " << setw(6) << x;
        ss << "  " << setw(6) << y;
        ss << "  " << setw(5) << z;
        ss << "  " << setw(5) << dx;
        ss << "  " << setw(5) << dy;
        ss << "  " << setw(3) << dz;
        ss << "  " << setw(3) << fl;
        return ss.str();
    }

    // overloaded stream insertion operator (see below)
    friend ostream &operator<<(ostream &out, const PlaneInfo_t &info);
};

// stream insertion operator for PlaneInfo_t
inline ostream &operator<<(ostream &out, const PlaneInfo_t &info) {
    out << info.toString();
    return out;
}

// message type for IPC message passing between tasks
struct Msg {

	_pulse hdr;			// QNX pulse with type, subtype, code, etc.

	PlaneInfo_t info;	// custom plane info

    // union of float and int, only 1 can be used at a time
    union {
    	double doubleValue;	// for passing floating point data
    	int intValue;		// for pass integer data
    };
};

/****************************** ENUMS ******************************/

// messages types and subtypes for IPC message passing
enum MsgType : _Uint16t {
	// add msgs here
	TIMEOUT,
	RADAR,
	COMMAND,
	ALERT,
	INFO,
	EXIT
};
enum MsgSubtype : _Uint16t {
	// add subtypes here
	CHANGE_SPEED,
	CHANGE_ALTITUDE,
	CHANGE_POSITION,
	CHANGE_WINDOW,
	REQ,
	REPLY
};

// load creation enum
enum Load {low, medium, high, overload};

/****************************** UTILITY ******************************/

// return random value in range lo...hi
inline int randRange(int lo, int hi) {
    return rand() % (hi - lo) + lo;
}

/************************* SHARED VARIABLES *************************/

// mutex for print/read screen Resource used by Display/Console tasks
extern pthread_mutex_t mtx;
