#include "Cpu.h"

void* computerThread(void* arg) {

	Cpu& cpu = *((Cpu*)arg);

	cpu.fd = creat(HISTORY_FILENAME, S_IRUSR | S_IWUSR | S_IXUSR);

	// create channel to listen on for this thread
	cpu.attach = name_attach(NULL, CPU_CHANNEL, 0);
	if (cpu.attach == NULL) {
		cout << "ERROR: CREATING CPU SERVER" << endl;
		pthread_exit(NULL);
	}

	cout << "Running CPU Thread. Checking for violations at interval n = " << cpu.n << "s" << endl;

	int saveCounter = 0;
	bool exit = false;

	while (!exit) {
		Msg msg;
		int rcvid = MsgReceive(cpu.attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == -1) break;
		if (!rcvid && (msg.hdr.code == _PULSE_CODE_DISCONNECT))	{ ConnectDetach(msg.hdr.scoid);		continue; }
		if (msg.hdr.type == _IO_CONNECT) 						{ MsgReply(rcvid, EOK, NULL, 0);	continue; }
		if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX)	{ MsgError(rcvid, ENOSYS);			continue; }

		switch (msg.hdr.type) {

		case MsgType::RADAR:
			MsgReply(rcvid, EOK, 0, 0);
			{
				int cnt = msg.hdr.subtype;
				vector<PlaneInfo_t> planes;

				for (int i = 0; i < cnt;) {
					int rcvid = MsgReceive(cpu.attach->chid, (void *)&msg, sizeof(msg), NULL);

					if (msg.hdr.type == MsgType::RADAR) {
						planes.push_back(msg.info); // add plane info to vector
						MsgReply(rcvid, EOK, 0, 0); // acknowledge the message
						i++;						// increment in this IF statement, only when a plane info has been receive and added to return vector
					}
				}

				sort(planes.begin(), planes.end(), [](PlaneInfo_t left, PlaneInfo_t right) {return left.fl < right.fl; });

				if (planes.empty())
					cpu.sendPlaneToDisplay(PlaneInfo_t{}, -1, msg.intValue);

				for (size_t i = 0; i < planes.size(); i++)
					cpu.sendPlaneToDisplay(planes[i], i, msg.intValue);

				if (++saveCounter >= 6) {
					saveCounter = 0;
					cpu.saveAirspace(planes);
				}

				cpu.checkViolations(planes);
			}

			break;

		case MsgType::COMMAND:
			MsgReply(rcvid, EOK, 0, 0);

			if (msg.hdr.subtype == MsgSubtype::CHANGE_WINDOW) {
				cpu.n = msg.intValue;
				cpu.sendWindowToDisplay();
			} else
				cpu.sendToComms(msg);

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

	name_detach(cpu.attach, 0);
	write(cpu.fd, "\n", sizeof("\n"));
	close(cpu.fd);
	pthread_exit(NULL);
}

Cpu::Cpu() {
	if (pthread_create(&thread, NULL, computerThread, (void*)this))
		cout << "ERROR: MAKING CPU THREAD" << endl;
}

int Cpu::join() {
	cout << "Joining CPU Thread" << endl;
	return pthread_join(thread, NULL);
}

void Cpu::saveAirspace(const vector<PlaneInfo_t>& planes) {
	char buff[128];
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "t=%d:\n", time);
	write(fd, buff, sizeof(buff));

	if (planes.empty()) {
		sprintf(buff, "\tNo Planes");
		write(fd, buff, sizeof(buff));
	}

	for (PlaneInfo_t p : planes) {
		sprintf(buff, "\t%s\n", p.toString().c_str());
		write(fd, buff, sizeof(buff));
	}
}

void Cpu::checkViolations(const vector<PlaneInfo_t>& planes) {
	for (size_t i = 0; i < planes.size(); i++) {
		for (size_t j = i + 1; j < planes.size(); j++) {

			for (int t = 0; t < n; t += 1) {
				PlaneInfo_t thisPlane = calculatePosition(planes[i], t);
				PlaneInfo_t nextPlane = calculatePosition(planes[j], t);

				if (notSafe(thisPlane, nextPlane)) {
					alertDisplay(thisPlane.id, nextPlane.id, t);
					break;
				}
			}
		}
	}
}

PlaneInfo_t Cpu::calculatePosition(const PlaneInfo_t& plane, int t) {
	PlaneInfo_t position = plane;
	position.x += position.dx * t;
	position.y += position.dy * t;
	position.z += position.dz * t;
	return position;
}

bool Cpu::notSafe(const PlaneInfo_t& thisPlane, const PlaneInfo_t& nextPlane) {
	// safe zone is box around plane with l=w=3000 and h=1000
	if (abs(thisPlane.x - nextPlane.x) > SAFEZONE_H) return false;
	if (abs(thisPlane.y - nextPlane.y) > SAFEZONE_H) return false;
	if (abs(thisPlane.z - nextPlane.z) > SAFEZONE_V) return false;
	return true;
}

void Cpu::sendPlaneToDisplay(PlaneInfo_t info, int i, int time){
	int coid = 0;

	// open channel to display thread
	if ((coid = name_open(DISPLAY_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO DISPLAY" << endl;
		return;
	}

	// create exit message
	Msg msg;
	msg.hdr.type = MsgType::RADAR;
	msg.hdr.subtype = i;
	msg.info = info;
	msg.intValue = time;

	// send exit message
	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	// close the channel
	name_close(coid);
}

void Cpu::alertDisplay(int id1, int id2, int t) {
	int coid = 0;

	// open channel to display thread
	if ((coid = name_open(DISPLAY_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO DISPLAY" << endl;
		return;
	}

	// create exit message
	Msg msg;
	msg.hdr.type = MsgType::ALERT;
	msg.info.id = id1;	// store first plane id in info.id
	msg.info.x = id2;	// store second in x (quick hack)
	msg.info.y = t;		// store time in y (quick hack)

	// send exit message
	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	// close the channel
	name_close(coid);
}

void Cpu::sendWindowToDisplay() {
	int coid = 0;

	if ((coid = name_open(DISPLAY_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO DISPLAY" << endl;
		return;
	}

	Msg msg;
	msg.hdr.type = MsgType::COMMAND;
	msg.hdr.subtype = MsgSubtype::CHANGE_WINDOW;
	msg.intValue = n;

	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	name_close(coid);
}

void Cpu::sendToComms(Msg msg) {
	int coid;
	if ((coid = name_open(COMMS_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO COMMS" << endl;
		return;
	}
	MsgSend(coid, &msg, sizeof(msg), 0, 0);
	name_close(coid);
}
