/*
	Console.cpp
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Console System Thread class.
		Handles input commands from Operator to CPU system + other systems.
 */
#include "Console.h" // class header

// Thread function. Uses void pointer arg to pass pointer to object calling the function
void* consoleThread(void* arg) {

	Console& console = *((Console*)arg); // store Console object that started thread

	// open file for logging operator commands
	int fd = creat(COMMANDS_FILENAME, S_IRUSR | S_IWUSR | S_IXUSR);

	cout << "Running Console Thread" << endl;

	string buffer = ""; // buffer stores current string being entered until '\n' detected
	deque<string> last; // small history of commands entered
	while (!console.exit) {

		delay(50); // small delay to help with console printing/reading bugs

		pthread_mutex_lock(&mtx);	// lock when editing screen
		unsigned char c = getch();	// non-blocking get char, returns ERR if nothing there
		pthread_mutex_unlock(&mtx);	// unlock after

		if (c == ERR)	// restart loop on ERR
			continue;

		// if newline entered, we want to try to run the command after parsing arguments
		if (c == '\n') {
            int x, y;	// to store cursor location on screen

            pthread_mutex_lock(&mtx);	// lock mutex so Display tasks cant make changes to screen
            getyx(stdscr, y, x);		// get the cursor position
            move(y, x - buffer.size());	// move it back to start of input
            for (size_t i = 0; i < buffer.size(); i++)
            	printw(" ");			// print ' ' to erase input
            pthread_mutex_unlock(&mtx); // unlock mutex

            // search start of string for recognized commands, then try to parse arguments if one is found
			if (buffer.find("changeWindow") != string::npos)		console.parseWindowCmd(buffer);
			else if (buffer.find("changeSpeed") != string::npos)	console.parseSpeedCmd(buffer);
			else if (buffer.find("changeAlt") != string::npos)		console.parseAltCmd(buffer);
			else if (buffer.find("changePos") != string::npos)		console.parsePosCmd(buffer);
			else if (buffer.find("info") != string::npos)			console.parseInfoCmd(buffer);
			else if (buffer.find("exit") != string::npos) {
				// premature exit entered by user
				clear();
				endwin();
				cout << "*** USER EXIT ***" << endl;
				exit(EXIT_SUCCESS);
			}
			else // command was not recognized
				buffer = "* INVALID CMD: " + buffer;

			// store input in FIFO stack max size 3
			if (last.size() >= 3) {
				last.pop_back();
				last.push_front(buffer);
			} else {
				last.push_front(buffer);
			}

			// log command to command.txt file
			console.saveCmd(fd, buffer);
			// clear the buffer
			buffer = "";
		}
		// backspace character detected (0x7F)
		else if (c == 0x7F) {
			size_t size = buffer.size(); // use current size
			if (size > 0) { // only delete if string has contents
				int y, x;

				pthread_mutex_lock(&mtx);	// lock I/O mutex
				getyx(stdscr, y, x);		// get cursor position
				mvprintw(y, x - 1, " ");	// move back 1 and print ' '
				move(y, x);					// move back after ' ' print
	            pthread_mutex_unlock(&mtx); // unlock I/O mutex

	            // remove the char from the buffer
				buffer = buffer.substr(0, size - 1);
			}
		}
		// put limit of 64 chars on input buffer
		else if (buffer.size() < 64) {
			// accepted chars for input string are space, minus, dot, and alphanumeric
			if ((c == ' ') || (c == '-') || (c == '.') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
				buffer += c; // add char to buffer
		}

		pthread_mutex_lock(&mtx); // lock I/O mutex
		// for loop handles updating short command history FIFO with newly entered command
		for (size_t i = 0; i < last.size(); i++) {
			move(48 - i, 2); // move to left of screen
			for (int j = 2; j < (getmaxx(stdscr) - ALERT_GAP) / 2; j++)
				printw(" "); // print ' ' until border
			mvprintw(48 - i, 2, "%s", last[i].c_str()); // print lastest command from bottom to top
		}
		move(52, 4); // move back to input location
		if (!buffer.empty()) // reprint string if not empty
			printw(buffer.c_str());
		refresh();	// refresh display (synchronous screen update)
		pthread_mutex_unlock(&mtx); // unlock I/O mutex
	}

	write(fd, "\n", sizeof("\n")); // extra newline to terminate file
	close(fd);	// close file
	clear();	// clear the screen
	endwin();	// end Ncurses
	delay(1000); // petit delay

	pthread_exit(NULL);
}

// construct starts thread and passes itself as arg
Console::Console() {
	if (pthread_create(&thread, NULL, consoleThread, (void *)this))
		cout << "ERROR: CREATING CONSOLE THREAD" << endl;
}

// join the thread from main(), also prints Finished message for user
int Console::join() {
	int r, c;
	string bar = "*******************************************"; // top bar
	string bor = "*                                         *"; // side border
	string msg = "*  Simulation over. Hit any key to exit.  *"; // text with border

	if (!exit) { // only show on normal exit
		exit = true;
		pthread_mutex_lock(&mtx); 	// lock I/O
		getmaxyx(stdscr, r, c); 	// screen limits
		mvprintw(r/2 - 2, (c - bar.size())/2, bar.c_str());	// top bar
		mvprintw(r/2 - 1, (c - bor.size())/2, bor.c_str());	// left/right borders
		mvprintw(r/2, (c - msg.size())/2, msg.c_str());		// message
		mvprintw(r/2 + 1, (c - bor.size())/2, bor.c_str());	// left/right borders
		mvprintw(r/2 + 2, (c - bar.size())/2, bar.c_str());	// bottom bar
		while(getch() == ERR); 		// loop until uses presses a key
		pthread_mutex_unlock(&mtx);	// unlock I/O
	}

    cout << "Joining Console Thread" << endl;
	return pthread_join(thread, NULL); // join thread
}

// parses CPU change window size command
void Console::parseWindowCmd(string& buffer) {
	stringstream ss(buffer); // use stringstream for ease of use
	string s; // string to store a word

	ss >> s; // ignore cmd name first word
	ss >> s; // get cmd arg 'n' for window

	try {
		int n = stoi(s); 				// try to get integer from string input for 'n'
		if (n < 5 || n > 180) throw ""; // n must be in value range
		changeWindow(n);				// send command to change window to CPU
	} catch (...) { // handle errors with message
		buffer = "* INVALID ARG: " + buffer + ". Accepted values: 5 <= n <= 180.";
	}
}

// parses Plane change speed command
void Console::parseSpeedCmd(string& buffer) {
	stringstream ss(buffer); // use stringstream for ease of use
	string s; // string to store a word

	ss >> s; // ignore cmd name first word
	ss >> s; // get cmd arg ID

	try {
		int id = stoi(s); 	// try to turn string into int
		ss >> s; 			// get cmd arg speed
		double v = stof(s); // convert to double
		// check for both data valid
		if ((id > 0 && id < 10000) && (v >= MIN_SPEED && v <= MAX_SPEED))
			changeSpeed(id, v); // send change speed command (CON -> CPU -> COMMS -> PLANE)
		else // otherwise an argument is out of range
			buffer = "* BAD ARG: " + buffer + ". Accepted values: 0 < id < 10000, 733 <= s <= 1026.";
	} catch (...) { // handle errors with message
		buffer = "* INVALID ARG(s): " + buffer + ". Requires ID & speed.";
	}
}

// parses Plane change altitude command
void Console::parseAltCmd(string& buffer) {
	stringstream ss(buffer); // use stringstream for ease of use
	string s; // string to store a word

	ss >> s; // ignore cmd name first word
	ss >> s; // get cmd arg ID

	try {
		int id = stoi(s); 	// try to turn string into int
		ss >> s; 			// get cmd arg ALT
		int alt = stoi(s);	// try to turn string into int
		// check for both data valid
		if ((id > 0 && id < 10000) && (alt >= MIN_Z && alt <= MAX_Z))
			changeAlt(id, alt);	// send change Altitude command (CON -> CPU -> COMMS -> PLANE)
		else // otherwise an argument is out of range
			buffer = "* BAD ARG: " + buffer + ". Accepted values: 0 < id < 10000, 15000 <= alt <= 40000.";
	} catch (...) { // handle errors with message
		buffer = "* INVALID ARG(s): " + buffer + ". Requires ID & altitude.";
	}
}

// parses Plane change position command
void Console::parsePosCmd(string& buffer) {
	stringstream ss(buffer); // use stringstream for ease of use
	string s; // string to store a word

	ss >> s; // ignore cmd name first word
	ss >> s; // get cmd arg ID

	try {
		int id = stoi(s); 		// try to turn string into int
		ss >> s;				// get cmd arg Angle
		float angle = stof(s);	// try to turn string to float
		// check for both data valid
		if ((id > 0 && id < 10000) && (angle >= -25 && angle <= 25))
			changePos(id, angle); // send change position command (CON -> CPU -> COMMS -> PLANE)
		else // otherwise an argument is out of range
			buffer = "* BAD ARG: " + buffer + ". Accepted values: 0 < id < 10000, -25 <= angle <= 25.";
	} catch (...) { // handle errors with message
		buffer = "* INVALID ARG(s): " + buffer + ". Requires ID & angle.";
	}
}

// parses Plane info command
void Console::parseInfoCmd(string& buffer) {
	stringstream ss(buffer); // use stringstream for ease of use
	string s; // string to store a word

	ss >> s; // ignore cmd name first word
	ss >> s; // get cmd arg ID

	try {
		int id = stoi(s);			// try to turn string into int
		if (id > 0 && id < 10000)	// check for data valid
			dispInfo(id);			// send info command (CON -> CPU -> COMMS -> PLANE -> COMMS -> CPU -> DISPLAY)
		else // otherwise an argument is out of range
			buffer = "* BAD ARG: " + buffer + ". Accepted values: 0 < id < 10000.";
	} catch (...) { // handle errors with message
		buffer = "* INVALID ARG(s): " + buffer + ". Requires ID.";
	}
}

// send the change window command to CPU
void Console::changeWindow(int n) {
	// create channel to CPU
	int coid;
	if ((coid = name_open(CPU_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO CPU" << endl;
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

// send the change speed command
// goes through: CONSOLE -> CPU -> COMMS -> PLANE
void Console::changeSpeed(int id, double v) {
	// create channel to CPU
	int coid;
	if ((coid = name_open(CPU_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO COMMS" << endl;
		return;
	}

	// create the message
	Msg msg;
	msg.hdr.type = MsgType::COMMAND;
	msg.hdr.subtype = MsgSubtype::CHANGE_SPEED;
	msg.info.id = id;
	msg.doubleValue = v;

	// send message
	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	// close channel
	name_close(coid);
}

// send the change altitude command
// goes through: CONSOLE -> CPU -> COMMS -> PLANE
void Console::changeAlt(int id, int alt) {
	// create channel to CPU
	int coid;
	if ((coid = name_open(CPU_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO COMMS" << endl;
		return;
	}

	// create the message
	Msg msg;
	msg.hdr.type = MsgType::COMMAND;
	msg.hdr.subtype = MsgSubtype::CHANGE_ALTITUDE;
	msg.info.id = id;
	msg.info.z = alt;

	// send message
	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	// close channel
	name_close(coid);
}

// send the change position command
// goes through: CONSOLE -> CPU -> COMMS -> PLANE
void Console::changePos(int id, float angle) {
	// create channel to CPU
	int coid;
	if ((coid = name_open(CPU_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO COMMS" << endl;
		return;
	}

	// create the message
	Msg msg;
	msg.hdr.type = MsgType::COMMAND;
	msg.hdr.subtype = MsgSubtype::CHANGE_POSITION;
	msg.info.id = id;
	msg.doubleValue = angle;

	// send message
	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	// close channel
	name_close(coid);
}

// send the display info command
// goes through: CONSOLE -> CPU -> COMMS -> PLANE
// comes back: PLANE -> COMMS -> CPU -> DISPLAY
void Console::dispInfo(int id) {
	// create channel to CPU
	int coid;
	if ((coid = name_open(CPU_CHANNEL, 0)) == -1) {
		cout << "ERROR: CREATING CLIENT TO COMMS" << endl;
		return;
	}

	// create the message
	Msg msg;
	msg.hdr.type = MsgType::INFO;
	msg.hdr.subtype = MsgSubtype::REQ;
	msg.info.id = id;

	// send message
	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	// close channel
	name_close(coid);
}

// stores a command in the commands.txt history file
void Console::saveCmd(int fd, const string& buffer) {
	char buff[128]; // string buffer
	memset(buff, 0, sizeof(buff)); // clear buffer
	sprintf(buff, "%s\n", buffer.c_str()); // format string
	write(fd, buff, sizeof(buff)); // print string to file
}

// sends an exit command to given channel
void Console::sendExit(const char* channel) {
	// open channel to thread
	int coid = 0;
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
