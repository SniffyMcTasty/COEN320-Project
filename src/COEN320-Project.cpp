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

#include "Constants.h"
#include "LoadCreationAlgorithm.h"
#include "CpuSystem.h"
#include "Plane.h"
#include "Radar.h"
#include "common.h"
#include "Display.h"

using namespace std;

bool createInputFile();
vector<pair<int, PlaneInfo_t>> readInputFile();

void sendExit(const char* channel);

int main()
{
	cout << "***** APPLICATION START *****" << endl;


//    CpuSystem cpu;
//    Display disp;
//	vector<Plane*> airspace;
//    Radar radar(&airspace);
//
//    delay(3000);
//	airspace.push_back(new Plane(Plane::randomInfo()));
//
//    delay(3000);
//	airspace.push_back(new Plane(Plane::randomInfo()));
//
//    delay(3000);
//	airspace.push_back(new Plane(Plane::randomInfo()));
//
//    delay(3000);
//	airspace.push_back(new Plane(Plane::randomInfo()));
//
//    delay(3000);
//	airspace.push_back(new Plane(Plane::randomInfo()));
//
//    delay(3000);
//	airspace.push_back(new Plane(Plane::randomInfo()));
//
//
//    for (Plane* p : airspace) {
//    	p->join();
//    	delete p;
//    }
//
//    sendExit(CPU_CHANNEL);
//    cpu.join();
//
//    sendExit(DISPLAY_CHANNEL);
//    disp.join();


	srand((int) time(0));
	if( !createInputFile() ) return EXIT_FAILURE; // file directory

	vector<pair<int, PlaneInfo_t>> planeArrivals = readInputFile();

	cout << planeArrivals.size() << " planes read from file" << endl;
	for (size_t i = 0; i < planeArrivals.size(); i++) {
		cout << "t = " << to_string(planeArrivals[i].first) << " -> " << planeArrivals[i].second << endl;
	}


    // airspace tracks all planes
	vector<Plane*> airspace;
	CpuSystem cpu;
	delay(500);
	Display display;
	delay(500);
	Radar radar(&airspace); // radar thread started with reference to airspace
	delay(500);
	int time = 0;

	// wait for Radar setup to complete (cant ping planes before radar is setup)
//	while (!radar.setup);

    // keep looping while there is still planes yet to arrive,
    // or once all planes have arrive, keep looping until no more planes detected by radar
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

	// join every thread
	for (Plane* p : airspace) {
		p->join();
		delete p;
	}

	sendExit(CPU_CHANNEL);
	cpu.join();

	sendExit(DISPLAY_CHANNEL);
	display.join();

	sendExit(RADAR_CHANNEL);
	radar.join();

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
	if (sw != sizeof(char_buffer))
	{
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


void testPlaneCommand() {
	Plane plane(Plane::randomInfo());
	delay(1000);
	int coid = name_open(plane.channel.c_str(), 0);
	delay(5000);
	Msg msg;
	msg.hdr.type = MsgType::COMMAND;
	msg.hdr.subtype = MsgSubtype::CHANGE_POSITION;
	msg.floatValue1 = 5;
	MsgSend(coid, (void *)&msg, sizeof(msg), 0, 0);
	name_close(coid);
	plane.join();
}



void sendPlaneAlert(PlaneInfo_t info){
	int coid = 0;

		// open channel to radar thread
	    if ((coid = name_open(DISPLAY_CHANNEL, 0)) == -1)
	    	cout << "ERROR: CREATING CLIENT TO DISPLAY" << endl;

	    // create exit message
		Msg msg;
		msg.hdr.type = MsgType::ALERT;
		msg.info = info;

		// send exit message
		MsgSend(coid, &msg, sizeof(msg), 0, 0);

		// close the channel
		name_close(coid);
}

