#include "Display.h"

#define COL_START 13
#define ROW_START 6

void* displayThread(void* arg){


	Display& display = *((Display*)arg);
	display.setupChannel();

	cout << "Running Display Thread" << endl;

	initscr();
	nodelay(stdscr, true);
	noecho();
	getmaxyx(stdscr, display.rows, display.cols);
	display.makeBorders();

	int rowRadar = 0, colRadar = 0, rowAlert = 4, cnt = 0;
	bool exit = false;
	while (!exit) {
		Msg msg;

		int rcvid = MsgReceive(display.attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == -1) break;
		if (!rcvid && (msg.hdr.code == _PULSE_CODE_DISCONNECT))	{ ConnectDetach(msg.hdr.scoid);		continue; }
		if (msg.hdr.type == _IO_CONNECT) 						{ MsgReply(rcvid, EOK, NULL, 0);	continue; }
		if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX)	{ MsgError(rcvid, ENOSYS);			continue; }

		switch (msg.hdr.type) {

		case MsgType::COMMAND:
			MsgReply(rcvid, EOK, 0, 0);

			pthread_mutex_lock(&mtx);
			getyx(stdscr, display.r, display.c);

			mvprintw(display.rows - 7, display.cols - ALERT_GAP + 4, "Violation Window: n=%03d", msg.intValue);

			move(display.r, display.c);
			refresh();
			pthread_mutex_unlock(&mtx);
			break;

		case MsgType::RADAR:
			MsgReply(rcvid, EOK, 0, 0);
			pthread_mutex_lock(&mtx);
			getyx(stdscr, display.r, display.c);
			if ((short)msg.hdr.subtype <= 0) {
				cnt = 0;
				mvprintw(4, COL_START, TITLES);
				mvprintw(4, COL_START + (display.cols - ALERT_GAP) / 2, TITLES);
				for (int i = ROW_START; i < display.rows - 12; i++) {
					move(i, 1);
					for (int j = 0; j < (display.cols - ALERT_GAP) / 2 - 1; j++)
						printw(" ");
					move(i, (display.cols - ALERT_GAP) / 2 + 1);
					for (int j = 0; j < (display.cols - ALERT_GAP) / 2 - 1; j++)
						printw(" ");
				}
				rowRadar = 0;
				colRadar = 0;
				for (int i = 3; i < rowAlert + 1; i++) {
					move(i, display.cols - ALERT_GAP + 1);
					for (int j = 0; j < ALERT_GAP - 2; j++)
						printw(" ");
				}
				rowAlert = 4;
			}
			if ((short)msg.hdr.subtype == -1)
				mvprintw(ROW_START + 1, COL_START, "No Planes");
			else {
				if (colRadar == 2)
					mvprintw((display.rows - 12) / 2, (display.cols - ALERT_GAP)/2 - 8, "!!! OVERLOAD !!!");
				else {
					cnt++;
					mvprintw(ROW_START + rowRadar++, COL_START + colRadar * (display.cols - ALERT_GAP) / 2, "%s", msg.info.toString().c_str());
					if (rowRadar > 35) {
						rowRadar = 0;
						colRadar += 1;
					}
				}
			}
			mvprintw(2, 2, "[t=%d] Radar Results: %d", msg.intValue, cnt);
			move(display.r, display.c);
			refresh();
			pthread_mutex_unlock(&mtx);
			break;

		case MsgType::ALERT:
			MsgReply(rcvid, EOK, 0, 0);
			pthread_mutex_lock(&mtx);
			getyx(stdscr, display.r, display.c);
			// quick hack for msg parameters below: id1 is in id, id2 is in x, and time is in y
			mvprintw(rowAlert++, display.cols - (ALERT_GAP + 27)/2, "@ t+%03d : ID=%04d & ID=%04d", msg.info.y, msg.info.id, msg.info.x);

			//                cout << "\nMAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!!" << endl;
			//                cout << "            ______" << endl;
			//                cout << "            _\\ _--\\___" << endl;
			//                cout << "    =  = ==(____AA____D" << endl;
			//                cout << "                \\_____\\___________________,----------.._" << "\tSafety Violation:" << endl;
			//                cout << "                /     o o o o o o o o o o o o o o o o  |\\_" << "\tPlane ID-" << msg.info.id << endl;
			//                cout << "                `--.__        ___..----..                  )" << "\t& Plane ID-" << msg.info.x << endl;
			//                cout << "                      `-----\\___________/------------`````" << "\tin t = " << msg.info.y << "s" << endl;
			//                cout << "                      =  ===(_________D\n" << endl;
			//                cout << "MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!!\n" << endl;
			move(display.r, display.c);
			refresh();
			pthread_mutex_unlock(&mtx);
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

	display.destroyChannel();
	pthread_exit(NULL);
}


Display::Display() {
	if (pthread_create(&thread, NULL, displayThread, (void*)this))
		cout << "ERROR: MAKING Display THREAD" << endl;
}

int Display::join() {
	cout << "Joining Display Thread" << endl;
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

void Display::makeBorders() {

	for (int i = 1; i < rows - 1; i++) {
		mvprintw(i, 0, "|");
		mvprintw(i, cols - 1, "|");
	}

	for (int i = 1; i < rows - 5; i++)
		mvprintw(i, cols - ALERT_GAP, "|");

	for (int i = 1; i < rows - 12; i++)
		mvprintw(i, (cols - ALERT_GAP) / 2, "|");

	string alert = "*** ALERTS ***";
	mvprintw(2, cols - (ALERT_GAP + alert.size())/2, alert.c_str());

	move(0, 0);			for (int i = 0; i < cols; i++) printw("-");
	move(rows - 1, 0);	for (int i = 0; i < cols; i++) printw("-");
	move(rows-5, 0);	for (int i = 0; i < cols; i++) printw("-");
	move(rows-12, 0);	for (int i = 0; i < cols - ALERT_GAP + 1; i++) printw("-");

	mvprintw(2, 2, "[t=0]: Nothing yet");

	mvprintw(rows - 3, 2, "# ");
	move(rows - 3, 4);

	refresh();
}
