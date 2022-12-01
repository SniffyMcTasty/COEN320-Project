#include "InitialPlane.h"

InitialPlane::InitialPlane(int time, int id) {
	this->time = time;
	this->id = id;
	this->setParameters();
}

string InitialPlane::toString() {
	return to_string(this->time) + ", " + to_string(this->id) + ", " + to_string(this->x) + ", " + to_string(this->y) +
			", " + to_string(this->z) + ", " + to_string(this->vx) + ", " + to_string(this->vy) + ", " + to_string(this->vz);
}

void InitialPlane::setParameters() {
	// determine from which plane of the airspace the plane will come from
	int entering = rand() % 4;

	// determine angle of trajectory (45 to 135) in radians (0 parallel to plane of entering)
	double angle = (rand() % 91 + 45) * 3.1416 / 180;

	// determine total speed
	int speed = rand() % (SPEED_INTERVAL + 1) + MIN_SPEED;

	// set parameters
	switch(entering) {
	// entering where x == 0, y != 0 (along y axis, angle 0 towards origin)
	case 0:
		this->x = 0;
		this->vx = sin(angle) * speed;
		this->y = rand() % (WIDTH + 1);
		this->vy = cos(angle) * speed;
		break;
	// entering where x != 0, y == 0 (along x axis, angle 0 away from origin)
	case 1:
		this->x = rand() % (WIDTH + 1);
		this->vx = cos(angle) * speed;
		this->y = 0;
		this->vy = sin(angle) * speed;
		break;
	// entering at max x, y != 0 (opposite of and parallel to the y axis, angle 0 away from x axis)
	case 2:
		this->x = WIDTH;
		this->vx = sin(angle) * -speed;
		this->y = rand() % (WIDTH + 1);
		this->vy = cos(angle) * speed;
		break;
	// entering at x != 0, max y (opposite of and parallel to the x axis, angle 0 towards y axis)
	case 3:
		this->x = rand() % (WIDTH + 1);
		this->vx = cos(angle) * -speed;
		this->y = WIDTH;
		this->vy = sin(angle) * -speed;
		break;
	}

	// set constant z
	this->z = rand() % (HEIGHT + 1) + BOTTOM;
	this->vz = 0;
}
