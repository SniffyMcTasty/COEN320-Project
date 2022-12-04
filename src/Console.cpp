#include "Console.h"

void* consoleThread(void* arg) {

	Console& console = *((Console*)arg);

	int fd = creat(COMMANDS_FILENAME, S_IRUSR | S_IWUSR | S_IXUSR);

	cout << "Running Console Thread" << endl;

	string buffer = "";
	deque<string> last;
	while (!console.exit) {

		delay(50); // small delay to help with console printing/reading bugs

		pthread_mutex_lock(&mtx);	// lock when editing screen
		unsigned char c = getch();	// non-blocking get char, returns ERR if nothing there
		pthread_mutex_unlock(&mtx);	// unlock after

		if (c == ERR)	// restart loop on ERR
			continue;

		if (c == '\n') {	// if newline entered, we want to
            int x, y;

            pthread_mutex_lock(&mtx);
            getyx(stdscr, y, x);
            move(y, x - buffer.size());
            for (size_t i = 0; i < buffer.size(); i++)
            	printw(" ");
            pthread_mutex_unlock(&mtx);

			if (buffer.find("changeWindow") != string::npos)
				console.parseWindowCmd(buffer);
			else if (buffer.find("changeSpeed") != string::npos)
				console.parseSpeedCmd(buffer);
			else if (buffer.find("changeAlt") != string::npos)
				console.parseAltCmd(buffer);
			else if (buffer.find("changePos") != string::npos)
				console.parsePosCmd(buffer);
			else if (buffer.find("exit") != string::npos) {
				console.mainExit = true;
				console.exit = true;
			}
			else
				buffer = "* INVALID CMD: " + buffer;

			if (last.size() >= 4) {
				last.pop_back();
				last.push_front(buffer);
			} else {
				last.push_front(buffer);
			}

			console.saveCmd(fd, buffer);
			buffer = "";
		}
		else if (c == 0x7F) {
			size_t size = buffer.size();
			if (size > 0) {
				int y, x;

				pthread_mutex_lock(&mtx);
				getyx(stdscr, y, x);
				mvprintw(y, x - 1, " ");
				move(y, x);
	            pthread_mutex_unlock(&mtx);

				buffer = buffer.substr(0, size - 1);
			}
		}
		else if ((c == ' ') || (c == '-') || (c == '.') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
			buffer += c;

		pthread_mutex_lock(&mtx);
		for (size_t i = 0; i < last.size(); i++) {
			move(48 - i, 2);
			for (int j = 2; j < getmaxx(stdscr) - ALERT_GAP; j++)
				printw(" ");
			mvprintw(48 - i, 2, "%s", last[i].c_str());
		}
		move(52, 4);
		if (!buffer.empty())
			printw(buffer.c_str());
		refresh();
		pthread_mutex_unlock(&mtx);
	}

	write(fd, "\n", sizeof("\n"));
	close(fd);
	clear();
	endwin();
	delay(1000);

	pthread_exit(NULL);
}

Console::Console(bool& exit) : mainExit{ exit } {
	if (pthread_create(&thread, NULL, consoleThread, (void *)this))
		cout << "ERROR: CREATING CONSOLE THREAD" << endl;
}

int Console::join() {
	int r, c;
	string bar = "*******************************************";
	string bor = "*                                         *";
	string end = "*  Simulation over. Hit any key to exit.  *";

	if (!exit) {
		exit = true;
		pthread_mutex_lock(&mtx);
		getmaxyx(stdscr, r, c);
		mvprintw(r/2 - 2, (c - bar.size())/2, bar.c_str());
		mvprintw(r/2 - 1, (c - bor.size())/2, bor.c_str());
		mvprintw(r/2, (c - end.size())/2, end.c_str());
		mvprintw(r/2 + 1, (c - bor.size())/2, bor.c_str());
		mvprintw(r/2 + 2, (c - bar.size())/2, bar.c_str());
		while(getch() == ERR);
		pthread_mutex_unlock(&mtx);
	}

    cout << "Joining Console Thread" << endl;
	return pthread_join(thread, NULL);
}

void Console::parseWindowCmd(string& buffer) {
	stringstream ss(buffer);
	string s;
	ss >> s; // ignore cmd name first word
	ss >> s; // get cmd arg 'n' for window
	try {
		int n = stoi(s);
		if (n < 5 || n > 180) throw "";
		changeWindow(n);
	} catch (...) {
		buffer = "* INVALID ARG: " + buffer + ". Accepted values: 5 <= n <= 180.";
	}
}

void Console::parseSpeedCmd(string& buffer) {
	stringstream ss(buffer);
	string s;
	ss >> s; // ignore cmd name first word
	ss >> s; // get cmd arg ID
	try {
		int id = stoi(s);
		ss >> s; // get cmd arg %
		float percent = stof(s);
		if ((id > 0 && id < 10000) && (percent >= -0.5 && percent <= 0.5))
			changeSpeed(id, percent);
		else
			buffer = "* BAD ARG: " + buffer + ". Accepted values: 0 < id < 10000, -0.5 <= \% <= 0.5.";
	} catch (...) {
		buffer = "* INVALID ARG(s): " + buffer + ". Requires ID & percentage.";
	}
}

void Console::parseAltCmd(string& buffer) {
	stringstream ss(buffer);
	string s;
	ss >> s; // ignore cmd name first word
	ss >> s; // get cmd arg ID
	try {
		int id = stoi(s);
		ss >> s; // get cmd arg ALT
		int alt = stoi(s);
		if ((id > 0 && id < 10000) && (alt >= MIN_Z && alt <= MAX_Z))
			changeAlt(id, alt);
		else
			buffer = "* BAD ARG: " + buffer + ". Accepted values: 0 < id < 10000, 15000 <= alt <= 40000.";
	} catch (...) {
		buffer = "* INVALID ARG(s): " + buffer + ". Requires ID & altitude.";
	}
}

void Console::parsePosCmd(string& buffer) {
	stringstream ss(buffer);
	string s;
	ss >> s;
	ss >> s;
	try {
		int id = stoi(s);
		ss >> s;
		float angle = stof(s);
		if ((id > 0 && id < 10000) && (angle >= -25 && angle <= 25))
			changePos(id, angle);
		else
			buffer = "* BAD ARG: " + buffer + ". Accepted values: 0 < id < 10000, -25 <= angle <= 25.";
	} catch (...) {
		buffer = "* INVALID ARG(s): " + buffer + ". Requires ID & angle.";
	}
}

void Console::changeWindow(int n) {
	int coid;

	if ((coid = name_open(CPU_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO CPU" << endl;
		return;
	}

	Msg msg;
	msg.hdr.type = MsgType::COMMAND;
	msg.hdr.subtype = MsgSubtype::CHANGE_WINDOW;
	msg.intValue = n;

	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	name_close(coid);
}

void Console::changeSpeed(int id, float percent) {
	int coid;
	if ((coid = name_open(CPU_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO COMMS" << endl;
		return;
	}
	Msg msg;
	msg.hdr.type = MsgType::COMMAND;
	msg.hdr.subtype = MsgSubtype::CHANGE_SPEED;
	msg.info.id = id;
	msg.floatValue = percent;
	MsgSend(coid, &msg, sizeof(msg), 0, 0);
	name_close(coid);
}

void Console::changeAlt(int id, int alt) {
	int coid;
	if ((coid = name_open(CPU_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO COMMS" << endl;
		return;
	}
	Msg msg;
	msg.hdr.type = MsgType::COMMAND;
	msg.hdr.subtype = MsgSubtype::CHANGE_ALTITUDE;
	msg.info.id = id;
	msg.info.z = alt;
	MsgSend(coid, &msg, sizeof(msg), 0, 0);
	name_close(coid);
}

void Console::changePos(int id, float angle) {
	int coid;
	if ((coid = name_open(CPU_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO COMMS" << endl;
		return;
	}
	Msg msg;
	msg.hdr.type = MsgType::COMMAND;
	msg.hdr.subtype = MsgSubtype::CHANGE_POSITION;
	msg.info.id = id;
	msg.floatValue = angle;
	MsgSend(coid, &msg, sizeof(msg), 0, 0);
	name_close(coid);
}

void Console::saveCmd(int fd, const string& buffer) {
	char buff[128];
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s\n", buffer.c_str());
	write(fd, buff, sizeof(buff));
}

void Console::sendExit(const char* channel) {
	int coid = 0;

	// open channel to thread
    if ((coid = name_open(channel, 0)) == -1) {
    	cout << "ERROR: CREATING CLIENT TO EXIT" << endl;
    	return;
    }

	// create exit message
	Msg msg;
	msg.hdr.type = MsgType::EXIT;

	// send exit message
	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	// close the channel
	name_close(coid);
}
