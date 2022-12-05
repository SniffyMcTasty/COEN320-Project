/*
	Cpu.cpp
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Computer System Thread class.
		Receives radar data, sends to display.
		Computes airspace violations from radar data.
		Routes IPC commands between systems.
 */
#include "Cpu.h" // class header

// Thread function. Uses void pointer arg to pass pointer to object calling the function
void* computerThread(void* arg) {

	Cpu& cpu = *((Cpu*)arg);	// store Cpu object that started thread
	cpu.setup();	// open file and setup IPC channel

	cout << "Running CPU Thread. Checking for violations at interval n = " << cpu.n << "s" << endl;

	int saveCounter = 0; // when saveCounter == 6, time interval is 6 * 5 = 30 secs
	bool exit = false;   // exit flag
	while (!exit) {
		Msg msg;
		int rcvid = MsgReceive(cpu.attach->chid, &msg, sizeof(msg), NULL); // block and wait for msg

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

		// react to incoming radar data
		case MsgType::RADAR:
			MsgReply(rcvid, EOK, 0, 0);
			// code block to fix compiler switch-case error
			{
				int cnt = msg.hdr.subtype;	// total incoming plane data stored in subtype
				vector<PlaneInfo_t> planes;	// vectors with plane infos

				// loop for the amount of expected radar plane infos
				for (int i = 0; i < cnt;) {
					// wait for message
					int rcvid = MsgReceive(cpu.attach->chid, (void *)&msg, sizeof(msg), NULL);

					// if its a radar message, add it to vector and EOK the message
					if (msg.hdr.type == MsgType::RADAR) {
						planes.push_back(msg.info); // add plane info to vector
						MsgReply(rcvid, EOK, 0, 0); // acknowledge the message
						i++;						// increment in this IF statement, only when a plane info has been receive and added to return vector
					}
				}

				// sort planes according to flight level user std::sort and lambda for PlaneInfo_t
				sort(planes.begin(), planes.end(), [](PlaneInfo_t left, PlaneInfo_t right) {return left.fl < right.fl; });

				// send special -1 code when no plane detected
				if (planes.empty()) cpu.sendPlaneToDisplay(PlaneInfo_t{}, -1, msg.intValue);

				// send all planes to the data display (include time in intValue param)
				for (size_t i = 0; i < planes.size(); i++)
					cpu.sendPlaneToDisplay(planes[i], i, msg.intValue);

				// at 30s intervals save planes in airspace
				if (++saveCounter >= 6) {
					saveCounter = 0;
					cpu.saveAirspace(planes, msg.intValue);
				}

				// check for constraint violations
				cpu.checkViolations(planes);
			}

			break;

		// handle change window commands, send all other commands from CONSOLE to COMMS
		case MsgType::COMMAND:
			MsgReply(rcvid, EOK, 0, 0);

			// change window command first
			if (msg.hdr.subtype == MsgSubtype::CHANGE_WINDOW) {
				cpu.n = msg.intValue;		// update congestion window n
				cpu.sendWindowToDisplay();	// send new value to display
			} else
				cpu.sendToComms(msg); // send other commands to comms

			break;

		// handle passing plane info req/reply back n forth
		case MsgType::INFO:
			MsgReply(rcvid, EOK, 0, 0);
			if (msg.hdr.subtype == MsgSubtype::REQ) // going from console to plane
				cpu.sendToComms(msg);
			else if (msg.hdr.subtype == MsgSubtype::REPLY) // going from plane to display
				cpu.sendToDisplay(msg);
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

	cpu.destroy();
	pthread_exit(NULL);
}

// constructor
Cpu::Cpu() {
	if (pthread_create(&thread, NULL, computerThread, (void*)this))
		cout << "ERROR: MAKING CPU THREAD" << endl;
}

// join the thread from main()
int Cpu::join() {
	cout << "Joining CPU Thread" << endl;
	return pthread_join(thread, NULL);
}

// setup output file and IPC server channel
void Cpu::setup() {
	// open output file
	fd = creat(HISTORY_FILENAME, S_IRUSR | S_IWUSR | S_IXUSR);
	// create channel to listen on for this thread
	attach = name_attach(NULL, CPU_CHANNEL, 0);
	if (attach == NULL) {
		cout << "ERROR: CREATING CPU SERVER" << endl;
		pthread_exit(NULL);
	}
}

// close input file and IPC channel
void Cpu::destroy() {
	name_detach(attach, 0);			// close IPC server
	write(fd, "\n", sizeof("\n"));	// end file with newline
	close(fd);						// close file
}

// stores the planes in history file with time
void Cpu::saveAirspace(const vector<PlaneInfo_t>& planes, int time) {
	char buff[128];					// string buffer
	memset(buff, 0, sizeof(buff));	// clear buffer

	sprintf(buff, "t=%d:\n", time);	// format string for time
	write(fd, buff, sizeof(buff));	// write time string to file

	// if there is no planes
	if (planes.empty()) {
		sprintf(buff, "\tNo Planes");
		write(fd, buff, sizeof(buff));
	}

	// write all planes to file when there is some
	for (PlaneInfo_t p : planes) {
		sprintf(buff, "\t%s\n", p.toString().c_str()); // format string
		write(fd, buff, sizeof(buff)); // write to file
	}
}

// checks all planes for Constraints Violations of w=l=3000 and h=1000
// during interval [t,t+n] with window 'n'
void Cpu::checkViolations(const vector<PlaneInfo_t>& planes) {

	// start with first planes
	for (size_t i = 0; i < planes.size(); i++) {
		// for the rest of planes (i + 1)
		for (size_t j = i + 1; j < planes.size(); j++) {

			// check future positions until 'n'
			for (int t = 0; t < n; t += 1) {
				PlaneInfo_t thisPlane = calculatePosition(planes[i], t);
				PlaneInfo_t nextPlane = calculatePosition(planes[j], t);

				// send alert if there will be violation
				if (notSafe(thisPlane, nextPlane)) {
					alertDisplay(thisPlane.id, nextPlane.id, t);
					break; // break so wont repeat times for same plane
				}
			}
		}
	}
}

// calculates future position based on current position, velocities and given time
PlaneInfo_t Cpu::calculatePosition(const PlaneInfo_t& plane, int t) {
	PlaneInfo_t position = plane;	// copy plane
	position.x += position.dx * t;	// calculate future x
	position.y += position.dy * t;	// calculate future y
	position.z += position.dz * t;	// calculate future z
	return position;
}

// compares plane positions to see if minimum horizontal separation of 3000
// and vertical separation of 1000 is respected
bool Cpu::notSafe(const PlaneInfo_t& thisPlane, const PlaneInfo_t& nextPlane) {
	// safe zone is box around plane with l=w=3000 and h=1000
	if (abs(thisPlane.x - nextPlane.x) > SAFEZONE_HORIZONTAL) return false;
	if (abs(thisPlane.y - nextPlane.y) > SAFEZONE_HORIZONTAL) return false;
	if (abs(thisPlane.z - nextPlane.z) > SAFEZONE_VERTICAL) return false;
	return true;
}

// send plane data to display task (from radar)
void Cpu::sendPlaneToDisplay(PlaneInfo_t info, int i, int time){
	// open channel to display thread
	int coid = 0;
	if ((coid = name_open(DISPLAY_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO DISPLAY" << endl;
		return;
	}

	// create radar message
	Msg msg;
	msg.hdr.type = MsgType::RADAR;
	msg.hdr.subtype = i;	// index of plane in list (not id)
	msg.info = info;		// save info
	msg.intValue = time;	// send time too

	// send exit message
	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	// close the channel
	name_close(coid);
}

// send alert with plane ids and tme to display when violation detected
void Cpu::alertDisplay(int id1, int id2, int t) {
	// open channel to display thread
	int coid = 0;
	if ((coid = name_open(DISPLAY_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO DISPLAY" << endl;
		return;
	}

	// create alert message
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

// send new violation constraints window to display thread
void Cpu::sendWindowToDisplay() {
	// create channel to Display
	int coid = 0;
	if ((coid = name_open(DISPLAY_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO DISPLAY" << endl;
		return;
	}

	// create the message
	Msg msg;
	msg.hdr.type = MsgType::COMMAND;
	msg.hdr.subtype = MsgSubtype::CHANGE_WINDOW;
	msg.intValue = n;

	// send message
	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	// close channel
	name_close(coid);
}

// used to forward plane info command result from comms system to display
void Cpu::sendToDisplay(Msg msg) {
	// create channel to Display
	int coid;
	if ((coid = name_open(DISPLAY_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO DISPLAY" << endl;
		return;
	}

	// forward the message
	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	// close the channel
	name_close(coid);
}

// used to forward info command from console system to comms system
void Cpu::sendToComms(Msg msg) {
	// create channel to Display
	int coid;
	if ((coid = name_open(COMMS_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO COMMS" << endl;
		return;
	}

	// forward the message
	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	// close the channel
	name_close(coid);
}
