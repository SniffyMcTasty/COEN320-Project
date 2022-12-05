/*
	COEN320-Project.cpp
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Main file for the ATC simulation project.
		Prompts user to enter plane load (low, medium, high, overload).
		Starts all threads, then releases planes according to their times.
		Waits to join threads and exit.
		Contains global (extern variables mtx (I/O mutex).
 */

// libraries
#include "common.h"
#include "Plane.h"
#include "LoadAlgo.h"
#include "Cpu.h"
#include "Radar.h"
#include "Console.h"
#include "Comms.h"
#include "Display.h"
using namespace std;

// function prototypes for functions used by main (see below main)
bool createInputFile();
vector<pair<int, PlaneInfo_t>> readInputFile();

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;	// shared mutex for printing/reading

int main() {
	cout << "***** APPLICATION START *****" << endl;

	srand((int) time(0));	// seed random generator

	// create the input file (asks user for load), exit program if it fails
	if(!createInputFile())
		return EXIT_FAILURE;

	// read the file into vector with arrival time (int) and the plane info
	vector<pair<int, PlaneInfo_t>> planeArrivals = readInputFile();

	// print planes for user to see
	cout << planeArrivals.size() << " planes read from file" << endl;
	for (size_t i = 0; i < planeArrivals.size(); i++)
		cout << "t = " << planeArrivals[i].first << " -> " << planeArrivals[i].second << endl;

	vector<Plane*> airspace; // airspace tracks all planes

	// start all system threads except planes
	Cpu cpu;
	Radar radar(&airspace); // radar thread started with reference to airspace so it can radar planes
	Console console;		// Operator console for entering commands
	Comms comms;			// Communications between planes and system (except radar)
	Display display;		// Display info on planes in airspace, constraint violations, and commands

	cpu.sendWindowToDisplay();	// send the initial constraints violation window to the Display thread

	int time = 0; // keep track of local time for plane releases
	while(!planeArrivals.empty()) {	// keep looping every second while there is still planes yet to arrive,

		// if the arrival time of the next plane is now
		if (time >= planeArrivals.front().first) {
			airspace.push_back(new Plane(planeArrivals.front().second)); // get next PlaneInfo from arrivals and create Plane thread
			planeArrivals.erase(planeArrivals.begin());	// delete info from arrivals after Plane started
		}

		delay(1000); // delay of 1s
		time++;	// advance time
	}

	// join every thread
	for (Plane* p : airspace) {
		p->join();
		delete p;
	}

	// send exit msgs so each thread stops MsgReceive() and quit while loops
	console.sendExit(RADAR_CHANNEL);
	console.sendExit(CPU_CHANNEL);
	console.sendExit(COMMS_CHANNEL);
	console.sendExit(DISPLAY_CHANNEL);

	// join all Threads
	console.join();
	cpu.join();
	comms.join();
	radar.join();
	display.join();

	cout << "***** APPLICATION END *****" << endl;
	return EXIT_SUCCESS;
}

// ask user for desired load (low, medium, high, overload) and create a random input
bool createInputFile()
{
	int fd;				  // file directory
	long unsigned int sw; // size written
	LoadAlgo algo;

	// open file with read, write, execute permissions and replace if existing
	fd = creat(INPUT_FILENAME, S_IRUSR | S_IWUSR | S_IXUSR);

	// prompt user for load
	string input;
	bool stop = false;

	// loop until user enters correct option and generate
	while (!stop) {
		cout << "Enter desired load (low, medium, high, overload): ";
		cin >> input;
		if (input.compare("low") == 0) {
			algo.createLoad(low);
		}
		else if (input.compare("medium") == 0) {
			algo.createLoad(medium);
		}
		else if (input.compare("high") == 0) {
			algo.createLoad(high);
		}
		else if (input.compare("overload") == 0) {
			algo.createLoad(overload);
		}
		else {
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

// reads the Plane load input file into vector of arrival times and plane info
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

	// parse and convert aguments to plane type
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
