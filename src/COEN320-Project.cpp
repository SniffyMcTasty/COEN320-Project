#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <utility>
#include <sstream>

// qcc -lang-c++ -Vgcc_ntox86_64 -c -Wp,-MMD,build/x86_64-debug/src/testNcurses.d,-MT,build/x86_64-debug/src/testNcurses.o -o build/x86_64-debug/src/testNcurses.o  -Wall -fmessage-length=0 -g -O0 -fno-builtin  src/testNcurses.cpp

#include "Constants.h"
#include "LoadCreationAlgorithm.h"
#include "Plane.h"
#include "Radar.h"
#include "common.h"
#include "Cpu.h"
#include "Display.h"
#include "Console.h"

using namespace std;

bool createInputFile();
vector<pair<int, PlaneInfo_t>> readInputFile();

void sendExit(const char* channel);
void parseAltCmd(string& input, int& id, int& alt);
void changeAlt(int id, int z);

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

int main()
{
	cout << "***** APPLICATION START *****" << endl;

	srand((int) time(0));

	if( !createInputFile() ) return EXIT_FAILURE; // file directory

//	vector<pair<int, PlaneInfo_t>> planeArrivals = readInputFile();
//
//	cout << planeArrivals.size() << " planes read from file" << endl;
//	for (size_t i = 0; i < planeArrivals.size(); i++) {
//		cout << "t = " << to_string(planeArrivals[i].first) << " -> " << planeArrivals[i].second << endl;
//	}

	vector<pair<int, PlaneInfo_t>> planeArrivals;
//	planeArrivals.push_back({0, {111, AIRSPACE_X / 2, 0, 25000, 0, 1000, 0, 300}});
	planeArrivals.push_back({0, {2222, AIRSPACE_X / 2, UPPER_Y, 30000, 0, 0, 500, 300}});

    // airspace tracks all planes
	vector<Plane*> airspace;
	Cpu cpu;
	delay(500);
	Radar radar(&airspace); // radar thread started with reference to airspace
	delay(500);
	Console console;
	delay(500);
	Display display;
	delay(500);
	int time = 0;

//     keep looping while there is still planes yet to arrive,
//     or once all planes have arrive, keep looping until no more planes detected by radar
	while(!planeArrivals.empty()) {

		// if there is still planes left to arrive
		if (!planeArrivals.empty()) {
			// if the arrival time of the next plane is now
			if (time >= planeArrivals.front().first) {
				airspace.push_back(new Plane(planeArrivals.front().second)); // get next PlaneInfo from arrivals and create Plane thread
				planeArrivals.erase(planeArrivals.begin());	// delete info from arrivals after Plane started
			}
		}

		// delay of 1s
		delay(1000);
		time++;
	}



//	while (true) {
//		string input = "";
//		getline(cin, input);
//
//		if (input.find("chAlt") != string::npos) {
//
//			int id;
//			int alt;
//
//			parseAltCmd(input, id, alt);
//
//			if (id < 10000 && (alt >= LOWER_Z && alt <= UPPER_Z)) {
//				cout << "Sending command: chAlt " << id << " " << alt << endl;
//				changeAlt(id, alt);
//			}
//			else
//				cout << "ERROR: bad command inputs" << endl;
//		}
//
//		if (input.find("exit") != string::npos) {
//			break;
//		}
//	}

	// join every thread
	for (Plane* p : airspace) {
		p->join();
		delete p;
	}

	console.join();

	sendExit(CPU_CHANNEL);
	cpu.join();

	sendExit(RADAR_CHANNEL);
	radar.join();

	sendExit(DISPLAY_CHANNEL);
	display.join();


	cout << "***** APPLICATION END *****" << endl;
	return EXIT_SUCCESS;
}

void sendExit(const char* channel) {
	int coid = 0;

	// open channel to thread
    if ((coid = name_open(channel, 0)) == -1)
    	cout << "ERROR: CREATING CLIENT TO RADAR" << endl;

	// create exit message
	Msg msg;
	msg.hdr.type = MsgType::EXIT;

	// send exit message
	MsgSend(coid, &msg, sizeof(msg), 0, 0);

	// close the channel
	name_close(coid);
}

bool createInputFile()
{
	int fd;				  // file directory
	long unsigned int sw; // size written
	LoadCreationAlgorithm algo;

	// open file with read, write, execute permissions and replace if existing
	fd = creat(INPUT_FILENAME, S_IRUSR | S_IWUSR | S_IXUSR);

	// prompt user for load
	string input;
	bool stop = false;

	while (!stop)
	{
		cout << "Enter desired load (low, medium, high): ";
		cin >> input;
		if (input.compare("low") == 0)
		{
			algo.createLoad(low);
		}
		else if (input.compare("medium") == 0)
		{
			algo.createLoad(medium);
		}
		else if (input.compare("high") == 0)
		{
			algo.createLoad(high);
		}
		else
		{
			cout << "Invalid input, try again." << endl;
			continue;
		}
		stop = true;
	}

	// write buffer to file
	string buffer = algo.getBuffer();
	char char_buffer[buffer.length() + 1];
	strcpy(char_buffer, buffer.c_str());
	sw = write(fd, char_buffer, sizeof(char_buffer));

	// test for error
	if (sw != sizeof(char_buffer)) {
		perror("Error writing loadInput.txt");
		return false;
	}
	cout << "Input load file created successfully" << endl;
	close(fd);

	return true;
}

vector<pair<int, PlaneInfo_t>> readInputFile()
{
	int sr;							   // size read
	int fd = open(INPUT_FILENAME, O_RDONLY); // file directory

	// read the file
	char buffer[100];
	string content;

	sr = read(fd, buffer, sizeof(buffer));
	content += buffer;
	while (sr == 100)
	{
		sr = read(fd, buffer, sizeof(buffer));
		content += buffer;
	}

	// close file directory
	close(fd);

	// create plane info vector from file content line by line
	vector<pair<int, PlaneInfo_t>> planes;
	stringstream ss(content);
	string line;
	getline(ss, line); // ignore first line
	while (getline(ss, line))
	{
		stringstream line_ss(line);
		string param;
		PlaneInfo_t info;

		getline(line_ss, param, ',');
		int release = stoi(param);

		getline(line_ss, param, ',');
		info.id = stoi(param);

		getline(line_ss, param, ',');
		info.x = stoi(param);

		getline(line_ss, param, ',');
		info.y = stoi(param);

		getline(line_ss, param, ',');
		info.z = stoi(param);

		getline(line_ss, param, ',');
		info.dx = stoi(param);

		getline(line_ss, param, ',');
		info.dy = stoi(param);

		getline(line_ss, param);
		info.dz = stoi(param);

		info.fl = info.z / 100;

		planes.push_back({release, info});
	}

	return planes;
}


void parseAltCmd(string& input, int& id, int& alt) {
	stringstream ss(input);
	string s;

	getline(ss, s, ' '); // skip cmd string

	getline(ss, s, ' '); // get id arg
	id = stoi(s);

	getline(ss, s, ' '); // get alt arg
	alt = stoi(s);
}

void changeAlt(int id, int z) {
	int coid;
	if ((coid = name_open(to_string(id).c_str(), 0)) == -1) {
		cout << "ERROR: CREATING CHANNEL TO PLANE-" << id << endl;
		return;
	}
	Msg msg;
	msg.hdr.type = MsgType::COMMAND;
	msg.hdr.subtype = MsgSubtype::CHANGE_ALTITUDE;
	msg.info.z = z;
	MsgSend(coid, (void *)&msg, sizeof(msg), 0, 0);
	name_close(coid);
}

