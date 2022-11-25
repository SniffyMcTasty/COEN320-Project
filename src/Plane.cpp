#include "Plane.h"

void *planeThread(void *arg) {

	Plane &plane = *((Plane *)arg);
	plane.setup();

	cout << "* NEW PLANE: " << plane << endl;

	Msg msg;
	while (plane.inZone()) {
		memset(&msg, 0, sizeof(msg));

		int rcvid = MsgReceive(plane.attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == -1) break;
		if (!rcvid && (msg.hdr.code == _PULSE_CODE_DISCONNECT))	{ ConnectDetach(msg.hdr.scoid);		continue; }
		if (msg.hdr.type == _IO_CONNECT) 						{ MsgReply(rcvid, EOK, NULL, 0);	continue; }
		if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX)	{ MsgError(rcvid, ENOSYS);			continue; }

		switch (msg.hdr.type) {

		case MsgType::RADAR:
			if (msg.hdr.subtype == MsgSubType::REQ) {
				// turn request msg into response msg with plane info and reply
				msg.hdr.subtype = MsgSubType::REPLY;
				msg.info = plane.info;
				MsgReply(rcvid, EOK, &msg, sizeof(msg));
				break;
			}

			break;

		case MsgType::TIMEOUT:
			plane.updatePosition();

		default:
			MsgReply(rcvid, EOK, 0, 0);
			break;
		}
	}

	plane.destroy();
	cout << "** PLANE EXIT: ID=" << plane.info.id << endl;
	pthread_exit(NULL);
}

ostream &operator<<(ostream &out, const Plane &plane) {
	return out << plane.toString();
}

Plane::Plane(PlaneInfo_t info) : info{info} {
	channel = to_string(info.id);
	if (pthread_create(&thread, NULL, planeThread, (void *)this))
		cout << "ERROR: MAKING PLANE THREAD" << endl;
}

int Plane::join() {
	return pthread_join(thread, NULL);
}

void Plane::setup() {
	setupChannel();
	setupTimer();
}

void Plane::setupChannel() {
	if ((attach = name_attach(NULL, channel.c_str(), 0)) == NULL) {
		cout << "ERROR: CREATING PLANE SERVER" << endl;
		pthread_exit(NULL);
	}

	if ((coid = name_open(channel.c_str(), 0)) == -1) {
		cout << "ERROR: CREATING PLANE CLIENT" << endl;
		pthread_exit(NULL);
	}
}

void Plane::setupTimer() {
	sigevent event;
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = coid;
	event.sigev_code = MsgType::TIMEOUT;

	if (timer_create(CLOCK_MONOTONIC, &event, &timerId) < 0) {
		cout << "ERROR: CREATING PLANE TIMER" << endl;
		pthread_exit(NULL);
	}

	itimerspec timerSpec{1, 0, 1, 0};

	if (timer_settime(timerId, 0, &timerSpec, NULL) < 0) {
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
	info.x += info.dx;
	info.y += info.dy;
	info.z += info.dz;
	info.fl = info.z / 100;
}

bool Plane::inZone() {
	return !(info.x < LOWER_X || info.x > UPPER_X || info.y < LOWER_Y || info.y > UPPER_Y || info.z < LOWER_Z || info.z > UPPER_Z);
}

string Plane::toString() const {
	return info.toString();
}

PlaneInfo Plane::ping() {
	Msg msg;
	msg.hdr.type = MsgType::RADAR;
	msg.hdr.subtype = MsgSubType::REQ;
	memset(&msg.info, 0, sizeof(msg.info));
	if (MsgSend(coid, &msg, sizeof(msg), &msg, sizeof(msg)) < 0)
		cout << "ERROR: PLANE PING FAILED" << endl;
	return msg.info;
}

PlaneInfo Plane::randomInfo() {
	//	- Aircraft enters airspace flying in horizontal plane (x or y plane) at
	//	constant velocity.
	//	- Maintains speed and altitude unless commanded to change.
	static int nextId = 1;
	int id, x, y, z, dx, dy, dz, fl;

	id = nextId++ * 55;

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
