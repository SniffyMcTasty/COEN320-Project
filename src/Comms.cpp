/*
	Comms.cpp
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Communications System Thread class.
		Handles commands sent from console to plane and back
 */
#include "Comms.h" // class header

// Thread function. Uses void pointer arg to pass pointer to object calling the function
void* commsThread(void* arg) {

	Comms& comms = *((Comms*)arg); // store Comms object that started thread
	comms.setup();	// setup IPC channel

	cout << "Running Comms Thread" << endl;
	bool exit = false;	// exit flag
	while (!exit) {
		Msg msg;
		int rcvid = MsgReceive(comms.attach->chid, &msg, sizeof(msg), NULL); // block and wait for msg

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

		// forward all commands type messages to planes
		case MsgType::COMMAND:
			MsgReply(rcvid, EOK, 0, 0);
			comms.send(msg);
			break;

		// forward Plane info commands back/forth from planes
		case MsgType::INFO:
			MsgReply(rcvid, EOK, 0, 0);
			if (msg.hdr.subtype == MsgSubtype::REQ)
				comms.send(msg);
			else if (msg.hdr.subtype == MsgSubtype::REPLY)
				comms.sendToCpu(msg);
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
	comms.destroy();
	pthread_exit(NULL);
}

// construct starts thread and passes itself as arg
Comms::Comms() {
	if (pthread_create(&thread, NULL, commsThread, (void*)this))
		cout << "ERROR: CREATING COMMS THREAD" << endl;
}

// join the thread from main()
int Comms::join() {
	cout << "Joining Comms Thread" << endl;
	return pthread_join(thread, NULL);
}

// setup IPC server channel
void Comms::setup() {
	if ((attach = name_attach(NULL, COMMS_CHANNEL, 0)) == NULL) {
		cout << "ERROR: CREATING COMMS SERVER" << endl;
		pthread_exit(NULL);
	}
}

// destroy IPC server channel
void Comms::destroy() {
	name_detach(attach, 0);
}

// send a message to a plane. ID is already in plane info in messsage
void Comms::send(Msg msg) {
	int coid;
	// open connection to plane
	if ((coid = name_open(to_string(msg.info.id).c_str(), 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO PLANE" << msg.info.id << endl;
		return;
	}
	// send the command
	MsgSend(coid, &msg, sizeof(msg), 0, 0);
	// close connection
	name_close(coid);
}

// echo a "info" command from plane back to CPU
void Comms::sendToCpu(Msg msg) {
	int coid;
	// open connection to plane
	if ((coid = name_open(CPU_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO CPU" << msg.info.id << endl;
		return;
	}
	// send the command
	MsgSend(coid, &msg, sizeof(msg), 0, 0);
	// close connection
	name_close(coid);
}
