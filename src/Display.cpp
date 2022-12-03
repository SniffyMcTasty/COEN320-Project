#include "Display.h"

void* displayThread(void* arg){

    Display& display = *((Display*)arg);

    display.setupChannel();

    cout<<"Display thread is a go!!!!!" << endl;

    bool exit = false;
	while (!exit) {
		Msg msg;

		int rcvid = MsgReceive(display.attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == -1) break;
		if (!rcvid && (msg.hdr.code == _PULSE_CODE_DISCONNECT))	{ ConnectDetach(msg.hdr.scoid);		continue; }
		if (msg.hdr.type == _IO_CONNECT) 						{ MsgReply(rcvid, EOK, NULL, 0);	continue; }
		if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX)	{ MsgError(rcvid, ENOSYS);			continue; }

		switch (msg.hdr.type) {

            case MsgType::PRINT:
                cout<<msg.info << endl;
                MsgReply(rcvid, EOK, 0, 0);
                break;

            case MsgType::ALERT:
                cout << "\nMAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!!" << endl;
                cout << "            ______" << endl;
                cout << "            _\\ _--\\___" << endl;
                cout << "    =  = ==(____AA____D" << endl;
                cout << "                \\_____\\___________________,----------.._" << "\tSafety Violation:" << endl;
                cout << "                /     o o o o o o o o o o o o o o o o  |\\_" << "\tPlane ID-" << msg.info.id << endl;
                cout << "                `--.__        ___..----..                  )" << "\t& Plane ID-" << msg.info.x << endl;
                cout << "                      `-----\\___________/------------`````" << "\tin t = " << msg.info.y << "s" << endl;
                cout << "                      =  ===(_________D\n" << endl;
                cout << "MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!!\n" << endl;
                MsgReply(rcvid, EOK, 0, 0);
                break;

            case MsgType::EXIT:
                cout << "Exiting Display" << endl;
                exit = true;
                MsgReply(rcvid, EOK, 0, 0);
                break;

		    default:
                MsgReply(rcvid, EOK, 0, 0);
                break;
            
        }
    }

    display.destroyChannel();

    pthread_exit(NULL);
}


Display::Display() {
	if (pthread_create(&thread, NULL, displayThread, (void*)this))
		cout << "ERROR: MAKING Display THREAD" << endl;
}

int Display::join() {
	return pthread_join(thread, NULL);
}


void Display::destroyChannel() {
	name_detach(attach, 0);
}

//Creating listener
void Display::setupChannel() {
	if ((attach = name_attach(NULL, DISPLAY_CHANNEL, 0)) == NULL) {
		cout << "ERROR: CREATING DISPLAY LISTNER" << endl;
		pthread_exit(NULL);
	}
}

