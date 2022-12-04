#include "Plane.h"

void *planeThread(void *arg) {

	Plane &plane = *((Plane *)arg);
	plane.setup();

	bool exit = false;
	while (plane.inZone() && !exit) {
		Msg msg;

		int rcvid = MsgReceive(plane.attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == -1) break;
		if (!rcvid && (msg.hdr.code == _PULSE_CODE_DISCONNECT))	{ ConnectDetach(msg.hdr.scoid);		continue; }
		if (msg.hdr.type == _IO_CONNECT) 						{ MsgReply(rcvid, EOK, NULL, 0);	continue; }
		if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX)	{ MsgError(rcvid, ENOSYS);			continue; }

		switch (msg.hdr.type) {

		case MsgType::RADAR:
			if (msg.hdr.subtype == MsgSubtype::REQ) {
				// turn request msg into response msg with plane info and reply
				msg.hdr.subtype = MsgSubtype::REPLY;
				msg.info = plane.info;
				MsgReply(rcvid, EOK, &msg, sizeof(msg));
			}
			break;

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
				const double &dx = plane.info.dx, &dy = plane.info.dy;
				double angle = atan(dy / dx);
				if (dx < 0) angle += PI;
				angle += msg.doubleValue * PI / 180;
				plane.info.dx = plane.v * cos(angle);
				plane.info.dy = plane.v * sin(angle);
			}
			break;

		case MsgType::TIMEOUT:
			MsgReply(rcvid, EOK, 0, 0);
			plane.updatePosition();
			break;

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

ostream &operator<<(ostream &out, const Plane &plane) {
	return out << plane.toString();
}

Plane::Plane(int time, int id) {
	this->time = time;
	this->info.id = id;
	this->setParameters();
}

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


string Plane::format() {
	stringstream ss;
	ss	<< time << ", " << info.id << ", "
		<< info.x << ", " << info.y << ", " << info.z << ", "
		<< info.dx << ", " << info.dy << ", " << info.dz;
	return ss.str();
}

Plane::Plane(PlaneInfo_t info) : info{info} {
	if (pthread_create(&thread, NULL, planeThread, (void *)this))
		cout << "ERROR: MAKING PLANE THREAD" << endl;
}

int Plane::join() {
	pthread_mutex_lock(&mtx);
	mvprintw(2, getcurx(stdscr) - ALERT_GAP - 18, "* Joining Plane Thread");
	refresh();
	pthread_mutex_unlock(&mtx);
	return pthread_join(thread, NULL);
}

void Plane::setup() {
	setupChannel();
	setupTimer();
}

void Plane::setupChannel() {
	if ((attach = name_attach(NULL, to_string(info.id).c_str(), 0)) == NULL) {
		cout << "ERROR: CREATING PLANE SERVER" << endl;
		pthread_exit(NULL);
	}

	if ((coid = name_open(to_string(info.id).c_str(), 0)) == -1) {
		cout << "ERROR: CREATING PLANE CLIENT" << endl;
		pthread_exit(NULL);
	}
}

void Plane::setupTimer() {
	sigevent event;
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = coid;
	event.sigev_code = MsgType::TIMEOUT;

	if (timer_create(CLOCK_MONOTONIC, &event, &timerId) < 0)
	{
		cout << "ERROR: CREATING PLANE TIMER" << endl;
		pthread_exit(NULL);
	}

	itimerspec timerSpec{1, 0, 1, 0};

	if (timer_settime(timerId, 0, &timerSpec, NULL) < 0)
	{
		cout << "ERROR: STARTING PLANE TIMER" << endl;
		pthread_exit(NULL);
	}
}

void Plane::destroy() {
	destroyTimer();
	destroyChannel();
}

void Plane::destroyTimer() {
	itimerspec off{0, 0, 0, 0};
	timer_settime(timerId, 0, &off, NULL);
	timer_delete(timerId);
}

void Plane::destroyChannel() {
	name_close(coid);
	name_detach(attach, 0);
}

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
	v = sqrt(info.dx * info.dx + info.dy * info.dy);
}

bool Plane::inZone() {
	return !(info.x < 0 || info.x > AIRSPACE_WIDTH || info.y < 0 || info.y > AIRSPACE_WIDTH || info.z < MIN_Z || info.z > MAX_Z);
}

string Plane::toString() const {
	return info.toString();
}

PlaneInfo_t Plane::ping() {
	Msg msg;
	msg.hdr.type = MsgType::RADAR;
	msg.hdr.subtype = MsgSubtype::REQ;
	memset(&msg.info, 0, sizeof(msg.info));
	if (MsgSend(coid, &msg, sizeof(msg), &msg, sizeof(msg)) < 0)
		cout << "ERROR: PLANE PING FAILED" << endl;
	return msg.info;
}

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
