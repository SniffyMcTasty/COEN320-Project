/*
	Display.cpp
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Display System Thread class.
		Handles printing stuff on the screen
 */
#include "Display.h" // class header

// Thread function. Uses void pointer arg to pass pointer to object calling the function
void* displayThread(void* arg) {

	Display& display = *((Display*)arg); // store Display object that started thread
	display.setupChannel();	// setup IPC channel

	cout << "Running Display Thread" << endl;

	// setup Ncurses
	initscr();				// initialize the screen
	nodelay(stdscr, true);	// non-blocking getch() function
	noecho();				// dont default echo characters on screen
	// get max screen dimensions
	getmaxyx(stdscr, display.maxRows, display.maxCols);
	display.makeBorders(); // make pretty borders for app

	// track rows and columns for Radar and Alert entries, also counter of radar results
	int rowRadar = 0, colRadar = 0, rowAlert = 4, radarResults = 0;
	bool exit = false; 	// exit flag
	deque<string> last; // small history of info commands results
	while (!exit) {
		Msg msg;
		int rcvid = MsgReceive(display.attach->chid, &msg, sizeof(msg), NULL);  // block and wait for msg

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

		// a plane info command result was received
		case MsgType::INFO:
			MsgReply(rcvid, EOK, 0, 0);

			// store input in FIFO stack max size 3
			if (last.size() >= 3) {
				last.pop_back();
				last.push_front(msg.info.toString());
			} else {
				last.push_front(msg.info.toString());
			}

			pthread_mutex_lock(&mtx);	// lock mutex for screen edit
			getyx(stdscr, display.r, display.c); // store current cursor row-col

			// print info command hist from FIFO queue
			for (size_t i = 0; i < last.size(); i++) {
				// first lines for erasing proper rows/cols
				move(48 - i, COL_START + (display.maxCols - ALERT_GAP) / 2);
				for (int j = 0; j < 51; j++)
					printw(" ");
				// print the info result
				mvprintw(48 - i, COL_START + (display.maxCols - ALERT_GAP) / 2, "%s", last[i].c_str());
			}

			move(display.r, display.c);	// move back to where the cursor was before print
			refresh();	// update screen
			pthread_mutex_unlock(&mtx);	// unlock I/O mutex

			break;


		// update violation window command received (only command that reaches display)
		case MsgType::COMMAND:
			MsgReply(rcvid, EOK, 0, 0);

			pthread_mutex_lock(&mtx); // lock I/O mutex
			getyx(stdscr, display.r, display.c); // store current cursor row-col

			// print the constraints violation window
			mvprintw(TITLE_ROW, display.maxCols - ALERT_GAP + 3, "* Violation Window: n=%03d *", msg.intValue);

			// reposition cursor and refresh
			move(display.r, display.c);
			refresh();
			pthread_mutex_unlock(&mtx); // unlock I/O mutex
			break;

		// handles receiving lists of radar replies
		case MsgType::RADAR:
			MsgReply(rcvid, EOK, 0, 0);

			// lock I/O mutex and save cursor position
			pthread_mutex_lock(&mtx);
			getyx(stdscr, display.r, display.c);

			// if subtype (cast as short for signed 16-bit value) is -1: NO PLANES, 0: First plane in batch, Other: just another plane
			if ((short)msg.hdr.subtype <= 0) {
				radarResults = 0; // reset total number of planes

				// print titles (this could be move somewhere else)
				mvprintw(TITLE_ROW, COL_START, TITLES);
				mvprintw(TITLE_ROW, COL_START + (display.maxCols - ALERT_GAP) / 2, TITLES);

				// erase columns of radar entries
				for (int i = 0; i < RADAR_ROWS_TOTAL; i++) {
					// left side
					move(RADAR_ROW_START + i, 1);
					for (int j = 0; j < (display.maxCols - ALERT_GAP) / 2 - 1; j++)
						printw(" ");
					// right side
					move(RADAR_ROW_START + i, (display.maxCols - ALERT_GAP) / 2 + 1);
					for (int j = 0; j < (display.maxCols - ALERT_GAP) / 2 - 1; j++)
						printw(" ");
				}
				rowRadar = 0;	// reset next print row to first one
				colRadar = 0;	// reset next print col to first col

				// alerts get erased on new batch of radar infos
				for (int i = ALERT_ROW_START; i < ALERT_ROW_START + rowAlert; i++) {
					move(i, display.maxCols - ALERT_GAP + 1);
					for (int j = 0; j < ALERT_GAP - 2; j++)
						printw(" ");
				}
				rowAlert = 0;	// reset next print row to first one
			}
			// check for no plane code and print message
			if ((short)msg.hdr.subtype == -1)
				mvprintw(RADAR_ROW_START + 1, COL_START, "No Planes");
			else {
				// else means we have planes, print all using left column, then right column until no more space
				if (colRadar >= 2)	// no more space!!
					mvprintw((display.maxRows - 12) / 2, (display.maxCols - ALERT_GAP)/2 - 8, "!!! OVERLOAD !!!");
				else {
					// increment number of radar results
					radarResults++;
					// print current radar plane data to screen at current row and col
					mvprintw(RADAR_ROW_START + rowRadar++, COL_START + colRadar * (display.maxCols - ALERT_GAP) / 2, "%s", msg.info.toString().c_str());
					// swap column at end of row
					if (rowRadar > RADAR_ROWS_TOTAL) {
						rowRadar = 0;
						colRadar += 1;
					}
				}
			}
			// show time of radar results and total results
			mvprintw(2, 2, "[t=%d] Radar Results: %d", msg.intValue, radarResults);
			move(display.r, display.c); // reset input cursor
			refresh(); 					// update screen
			pthread_mutex_unlock(&mtx);	// unlock I/O mutex
			break;

		// ALERT received about incoming plane violation
		case MsgType::ALERT:
			MsgReply(rcvid, EOK, 0, 0);

			// lock I/O mutex and save current cursor pos
			pthread_mutex_lock(&mtx);
			getyx(stdscr, display.r, display.c);

			// print the alert time (now + t) and both plane IDs
			// quick hack for msg parameters below:
			//	- id1 is in id
			//	- id2 is in x
			//	- and time is in y
			mvprintw(ALERT_ROW_START + rowAlert++, display.maxCols - (ALERT_GAP + 27)/2, "@ t+%03d : ID=%04d & ID=%04d", msg.info.y, msg.info.id, msg.info.x);

			// initially we wanted to print VERY visual alert
//			cout << "\nMAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!!" << endl;
//			cout << "            ______" << endl;
//			cout << "            _\\ _--\\___" << endl;
//			cout << "    =  = ==(____AA____D" << endl;
//			cout << "                \\_____\\___________________,----------.._" << "\tSafety Violation:" << endl;
//			cout << "                /     o o o o o o o o o o o o o o o o  |\\_" << "\tPlane ID-" << msg.info.id << endl;
//			cout << "                `--.__        ___..----..                  )" << "\t& Plane ID-" << msg.info.x << endl;
//			cout << "                      `-----\\___________/------------`````" << "\tin t = " << msg.info.y << "s" << endl;
//			cout << "                      =  ===(_________D\n" << endl;
//			cout << "MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!! MAYDAY!!!!!!!!!!!!!\n" << endl;

			move(display.r, display.c);	// reset cursor position
			refresh();					// update screen
			pthread_mutex_unlock(&mtx);	// unlock I/O
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

	display.destroyChannel();
	pthread_exit(NULL);
}

// construct starts thread and passes itself as arg
Display::Display() {
	if (pthread_create(&thread, NULL, displayThread, (void*)this))
		cout << "ERROR: MAKING Display THREAD" << endl;
}

// join thread from main
int Display::join() {
	cout << "Joining Display Thread" << endl;
	return pthread_join(thread, NULL);
}

// Creating IPC listener
void Display::setupChannel() {
	if ((attach = name_attach(NULL, DISPLAY_CHANNEL, 0)) == NULL) {
		cout << "ERROR: CREATING DISPLAY LISTNER" << endl;
		pthread_exit(NULL);
	}
}

// destroy IPC listener
void Display::destroyChannel() {
	name_detach(attach, 0);
}

// prints pretty borders on application start
void Display::makeBorders() {

	// print left/right border
	for (int i = 1; i < maxRows - 1; i++) {
		mvprintw(i, 0, "|");
		mvprintw(i, maxCols - 1, "|");
	}

	// print border for alerts on right
	for (int i = 1; i < maxRows - 5; i++)
		mvprintw(i, maxCols - ALERT_GAP, "|");

	// seperate remaining screen in 2 for radar data
	for (int i = 1; i < maxRows - 13; i++)
		mvprintw(i, (maxCols - ALERT_GAP) / 2, "|");

	// alert tile on right of screen
	string s = "A L E R T S";
	mvprintw(2, maxCols - (ALERT_GAP + s.size())/2, s.c_str());

	// print bars/rows to make boxes on screen
	move(0, 0);			for (int i = 0; i < maxCols; i++) printw("-");
	move(maxRows - 1, 0);	for (int i = 0; i < maxCols; i++) printw("-");
	move(maxRows-5, 0);	for (int i = 0; i < maxCols; i++) printw("-");
	move(maxRows-13, 0);	for (int i = 0; i < maxCols - ALERT_GAP + 1; i++) printw("-");

	// initial message
	mvprintw(2, 2, "[t=0]: Nothing yet");

	// subtitle
	s = "Recent Commands History:";
	mvprintw(maxRows - 11, (maxCols - ALERT_GAP) / 4 - s.size() / 2, s.c_str());

	// subtitle
	s = "Recent \'info\' Commands:";
	mvprintw(maxRows - 11, 3 * (maxCols - ALERT_GAP) / 4 - s.size() / 2, s.c_str());

	// position cursor for input
	mvprintw(maxRows - 3, 2, "# ");
	move(maxRows - 3, 4);

	refresh(); // update screen
}
