#include "Comms.h"

void* commsThread(void* arg) {

	Comms& comms = *((Comms*)arg);

	if ((comms.attach = name_attach(NULL, COMMS_CHANNEL, 0)) == NULL) {
		cout << "ERROR: CREATING COMMS SERVER" << endl;
		pthread_exit(NULL);
	}

	cout << "Running Comms Thread" << endl;

	bool exit = false;
	while (!exit) {
		Msg msg;
		int rcvid = MsgReceive(comms.attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == -1) break;
		if (!rcvid && (msg.hdr.code == _PULSE_CODE_DISCONNECT))	{ ConnectDetach(msg.hdr.scoid);		continue; }
		if (msg.hdr.type == _IO_CONNECT) 						{ MsgReply(rcvid, EOK, NULL, 0);	continue; }
		if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX)	{ MsgError(rcvid, ENOSYS);			continue; }

		switch (msg.hdr.type) {

		case MsgType::COMMAND:
			MsgReply(rcvid, EOK, 0, 0);
			comms.send(msg);
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

	name_detach(comms.attach, 0);
	pthread_exit(NULL);
}

Comms::Comms() {
	if (pthread_create(&thread, NULL, commsThread, (void*)this))
		cout << "ERROR: CREATING COMMS THREAD" << endl;
}

int Comms::join() {
	cout << "Joining Comms Thread" << endl;
	return pthread_join(thread, NULL);
}

void Comms::send(Msg msg) {
	int coid;
	if ((coid = name_open(to_string(msg.info.id).c_str(), 0)) == -1)
		cout << "ERROR: CREATING CLIENT TO PLANE " << msg.info.id << endl;
	MsgSend(coid, &msg, sizeof(msg), 0, 0);
	name_close(coid);
}
