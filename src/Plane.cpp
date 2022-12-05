/*
	Plane.cpp
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Plane Thread class.
		Simulates a plane, updates its position every second
		Responds to radar when in airspace
		Responds to commands from operator over comms system
 */
#include "Plane.h"

// Thread function. Uses void pointer arg to pass pointer to object calling the function
void *planeThread(void *arg) {

	Plane &plane = *((Plane *)arg);	// store Plane object that started thread
	plane.setup();	// setup timer and IPC

	bool exit = false; // exit flag
	while (plane.inZone() && !exit) {	// continue thread while still in the zone
		Msg msg;
		int rcvid = MsgReceive(plane.attach->chid, &msg, sizeof(msg), NULL);  // block and wait for msg

		// message receive error
		if (rcvid == -1) break;
		// client wants to disconnect
		if (!rcvid && (msg.hdr.code == _PULSE_CODE_DISCONNECT))	{ ConnectDetach(msg.hdr.scoid);		continue; }
		// new connection
		if (msg.hdr.type == _IO_CONNECT) 						{ MsgReply(rcvid, EOK, NULL, 0);	continue; }
		// other error
		if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX)	{ MsgError(rcvid, ENOSYS);			continue; }

		// expected message types
		switch (msg.hdr.type) {

		// radar req, formulate a response and send
		case MsgType::RADAR:
			if (msg.hdr.subtype == MsgSubtype::REQ) {
				// turn request msg into response msg with plane info and reply
				msg.hdr.subtype = MsgSubtype::REPLY;
				msg.info = plane.info; // send this plane info
				MsgReply(rcvid, EOK, &msg, sizeof(msg));
			}
			break;

		// respond to request for plane info from operator
		// DATA PATH: CONSOLE -> CPU -> COMMS -> PLANE,
		// then PLANE -> COMMS -> CPU -> DISPLAY
		case MsgType::INFO:
			MsgReply(rcvid, EOK, 0, 0);
			// make reply
			msg.hdr.subtype = MsgSubtype::REPLY;
			msg.info = plane.info;
			// open channel to comms
			int coid;
			if ((coid = name_open(COMMS_CHANNEL, 0)) == -1) {
				cout << "ERROR: CREATING CLIENT TO COMMS" << endl;
				break;
			}
			// send info
			MsgSend(coid, &msg, sizeof(msg), 0, 0);
			// close channel
			name_close(coid);
			break;

		// responde to different types of commands
		case MsgType::COMMAND:
			MsgReply(rcvid, EOK, 0, 0); // send the eok because it was blocked
			// message type command send by the the radar to make the plane change speed
			if (msg.hdr.subtype == MsgSubtype::CHANGE_SPEED) {
				double percent = (msg.doubleValue - plane.v) / plane.v;
				plane.info.dx *= (1 + percent);
				plane.info.dy *= (1 + percent);
				plane.v = msg.doubleValue;
			}
			// message type command send by the the radar to make the plane change altitude
			else if (msg.hdr.subtype == MsgSubtype::CHANGE_ALTITUDE) {
				plane.changeAltFlag = true;
				plane.finalAlt = msg.info.z;
				plane.info.dz = msg.info.z > plane.info.z ? +50 : -50;
			}
			// message type command send by the the radar to make the plane position
			else if (msg.hdr.subtype == MsgSubtype::CHANGE_POSITION) {
				// using trig to determine resultant directions using vector
				const double &dx = plane.info.dx, &dy = plane.info.dy;
				double angle = atan(dy / dx);
				if (dx < 0) angle += PI;
				angle += msg.doubleValue * PI / 180;
				plane.info.dx = plane.v * cos(angle);
				plane.info.dy = plane.v * sin(angle);
			}
			break;

		// update position on timer expiration
		case MsgType::TIMEOUT:
			MsgReply(rcvid, EOK, 0, 0);
			plane.updatePosition();
			break;

		// exit message
		case MsgType::EXIT:
			exit = true;
			MsgReply(rcvid, EOK, 0, 0);
			break;

		default:
			MsgReply(rcvid, EOK, 0, 0);
			break;
		}
	}

	plane.destroy();
	pthread_exit(NULL);
}

// output operator for plane class
ostream &operator<<(ostream &out, const Plane &plane) {
	return out << plane.toString();
}

// constructor with time and id for Input file generation
Plane::Plane(int time, int id) {
	this->time = time;
	this->info.id = id;
	this->setParameters(); // pseudo-random plane parameters
}

// generate pseudo-random plane parameters
void Plane::setParameters() {
	// determine from which plane of the airspace the plane will come from
	int entering = rand() % 4;

	// determine angle of trajectory (45 to 135) in radians (0 parallel to plane of entering)
	double angle = (rand() % 90 + 45) * PI / 180;

	// determine total speed
	int speed = rand() % SPEED_INTERVAL + MIN_SPEED;

	// set parameters
	switch(entering) {
	// entering where x == 0, y != 0 (along y axis, angle 0 towards origin)
	case 0:
		this->info.x = 0;
		this->info.dx = sin(angle) * speed;
		this->info.y = rand() % (AIRSPACE_WIDTH);
		this->info.dy = cos(angle) * speed;
		break;
	// entering where x != 0, y == 0 (along x axis, angle 0 away from origin)
	case 1:
		this->info.x = rand() % (AIRSPACE_WIDTH);
		this->info.dx = cos(angle) * speed;
		this->info.y = 0;
		this->info.dy = sin(angle) * speed;
		break;
	// entering at max x, y != 0 (opposite of and parallel to the y axis, angle 0 away from x axis)
	case 2:
		this->info.x = AIRSPACE_WIDTH;
		this->info.dx = sin(angle) * -speed;
		this->info.y = rand() % (AIRSPACE_WIDTH);
		this->info.dy = cos(angle) * speed;
		break;
	// entering at x != 0, max y (opposite of and parallel to the x axis, angle 0 towards y axis)
	case 3:
		this->info.x = rand() % (AIRSPACE_WIDTH);
		this->info.dx = cos(angle) * -speed;
		this->info.y = AIRSPACE_WIDTH;
		this->info.dy = sin(angle) * -speed;
		break;
	}

	// set constant z
	this->info.z = rand() % AIRSPACE_HEIGHT + MIN_Z;
	this->info.dz = 0;

	// set flight level
	this->info.fl = info.z / 100;
}

// print in format for input file
string Plane::format() {
	stringstream ss;
	ss	<< time << ", " << info.id << ", "
		<< info.x << ", " << info.y << ", " << info.z << ", "
		<< info.dx << ", " << info.dy << ", " << info.dz;
	return ss.str();
}

// constructor with PlaneInfo_t
Plane::Plane(PlaneInfo_t info) : info{info} {
	if (pthread_create(&thread, NULL, planeThread, (void *)this))
		cout << "ERROR: MAKING PLANE THREAD" << endl;
}

// join thread from main()
int Plane::join() {
	return pthread_join(thread, NULL);
}

// setup timer and channel
void Plane::setup() {
	setupChannel();
	setupTimer();
}

// setup IPC channels, 1 server to listen for msgs,
// the second to talk back to itself for timer rollover
// (also used for ping, easier to let planes handle channels than radar creating all the time)
void Plane::setupChannel() {
	// Server message listener
	if ((attach = name_attach(NULL, to_string(info.id).c_str(), 0)) == NULL) {
		cout << "ERROR: CREATING PLANE SERVER" << endl;
		pthread_exit(NULL);
	}

	// client message sender (for timer and radar ping)
	if ((coid = name_open(to_string(info.id).c_str(), 0)) == -1) {
		cout << "ERROR: CREATING PLANE CLIENT" << endl;
		pthread_exit(NULL);
	}
}

// setup timer for 1s intervals, to update plane position
void Plane::setupTimer() {
	// create signal
	sigevent event;
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = coid;
	event.sigev_code = MsgType::TIMEOUT; // timeout message code

	// create the timer
	if (timer_create(CLOCK_MONOTONIC, &event, &timerId) < 0) {
		cout << "ERROR: CREATING PLANE TIMER" << endl;
		pthread_exit(NULL);
	}

	// create timer specs of periodic 1 second timer
	itimerspec timerSpec{1, 0, 1, 0};

	// start timer according to specs
	if (timer_settime(timerId, 0, &timerSpec, NULL) < 0) {
		cout << "ERROR: STARTING PLANE TIMER" << endl;
		pthread_exit(NULL);
	}
}

// destroy timer and channel
void Plane::destroy() {
	destroyTimer();
	destroyChannel();
}

// turn off and delete timer
void Plane::destroyTimer() {
	itimerspec off{0, 0, 0, 0};
	timer_settime(timerId, 0, &off, NULL);
	timer_delete(timerId);
}

// close channels for IPC
void Plane::destroyChannel() {
	name_close(coid);
	name_detach(attach, 0);
}

// updates current position (called every second) base on current position and velocity
void Plane::updatePosition() {
	// setting a flage to see if the altedue is changing
	if (changeAltFlag && ((info.dz < 0 && info.z <= finalAlt) || (info.dz > 0 && info.z >= finalAlt))) {
		info.dz = 0;
		changeAltFlag = false;
	}

	info.x += info.dx;
	info.y += info.dy;
	info.z += info.dz;
	info.fl = info.z / 100;
	v = sqrt(info.dx * info.dx + info.dy * info.dy);	// also store magnitude of velocity
}

// check if plane is in the Airspace zone
bool Plane::inZone() {
	return !(info.x < 0 || info.x > AIRSPACE_WIDTH || info.y < 0 || info.y > AIRSPACE_WIDTH || info.z < MIN_Z || info.z > MAX_Z);
}

// convert plane to string (use its info)
string Plane::toString() const {
	return info.toString();
}

// Radar ping, returns position of aircraft
PlaneInfo_t Plane::ping() {
	Msg msg;
	msg.hdr.type = MsgType::RADAR;
	msg.hdr.subtype = MsgSubtype::REQ;
	memset(&msg.info, 0, sizeof(msg.info));
	if (MsgSend(coid, &msg, sizeof(msg), &msg, sizeof(msg)) < 0)
		cout << "ERROR: PLANE PING FAILED" << endl;
	return msg.info;
}

// status function which generates a random plane (utility/testing, not used)
PlaneInfo_t Plane::randomInfo() {
	//	- Aircraft enters airspace flying in horizontal plane (x or y plane) at
	//	constant velocity.
	//	- Maintains speed and altitude unless commanded to change.
	static int nextId = 1;
	int id, x, y, z, dx, dy, dz, fl;
	id = nextId++ * 55;
	if (rand() % 2) {
		x = randRange(AIRSPACE_WIDTH / 4, AIRSPACE_WIDTH - AIRSPACE_WIDTH / 4);
		y = rand() % 2 ? 0 : AIRSPACE_WIDTH;
		dx = 10 * randRange(-100, 100);
		dy = 10 * randRange(80, 120) * (y == 0 ? 1 : -1);
	}
	else {
		x = rand() % 2 ? 0 : AIRSPACE_WIDTH;
		y = randRange(AIRSPACE_WIDTH / 4, AIRSPACE_WIDTH - AIRSPACE_WIDTH / 4);
		dx = 10 * randRange(80, 120) * (x == 0 ? 1 : -1);
		dy = 10 * randRange(-100, 100);
	}
	z = randRange(MIN_Z + AIRSPACE_HEIGHT / 4, MAX_Z - AIRSPACE_HEIGHT / 4);
	dz = 0;
	fl = z / 100;
	return PlaneInfo_t{id, x, y, z, dx, dy, dz, fl};
}
