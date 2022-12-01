#ifndef INITIAL_PLANE_H
#define INITIAL_PLANE_H

#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <cmath>

#include "Constants.h"

using namespace std;

class InitialPlane {
private:
	int x, y, z; // initial x, y, z position
	int vx, vy, vz; // initial x, y, z speeds
	int id, time;
	void setParameters();

public:
	InitialPlane(int time, int id);
	string toString();
};

#endif
