
// TODO: Make exit command kill all planes
// TODO: plane info command

#include "common.h"
#include "Plane.h"
#include "LoadAlgo.h"
#include "Cpu.h"
#include "Radar.h"
#include "Console.h"
#include "Comms.h"
#include "Display.h"

using namespace std;

bool createInputFile();
vector<pair<int, PlaneInfo_t>> readInputFile();

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER; // shared mutex for printing/reading
vector<Plane*> airspace; // airspace tracks all planes

int main() {
	cout << "***** APPLICATION START *****" << endl;

	srand((int) time(0));

	if(!createInputFile())
		return EXIT_FAILURE;

	vector<pair<int, PlaneInfo_t>> planeArrivals = readInputFile();

	cout << planeArrivals.size() << " planes read from file" << endl;
	for (size_t i = 0; i < planeArrivals.size(); i++) {
		cout << "t = " << planeArrivals[i].first << " -> " << planeArrivals[i].second << endl;
	}

//	for (int t = 0; t < 200; t++)
//		planeArrivals.push_back({t, Plane::randomInfo()});

	bool exit = false;
	Cpu cpu;
	delay(25);
	Radar radar(&airspace); // radar thread started with reference to airspace
	delay(25);
	Console console(exit);
	delay(25);
	Comms comms;
	delay(25);
	Display display;
	delay(25);
	int time = 0;

	cpu.sendWindowToDisplay();

    // keep looping while there is still planes yet to arrive,
	while(!planeArrivals.empty() && !exit) {

		// if the arrival time of the next plane is now
		if (time >= planeArrivals.front().first) {
			airspace.push_back(new Plane(planeArrivals.front().second)); // get next PlaneInfo from arrivals and create Plane thread
			planeArrivals.erase(planeArrivals.begin());	// delete info from arrivals after Plane started
		}

		// delay of 1s
		delay(1000);
		time++;
	}

	// join every thread
	for (Plane* p : airspace) {
		if (exit && p->inZone())
			console.sendExit(p->getChannel());
		p->join();
		delete p;
	}

	console.sendExit(RADAR_CHANNEL);
	console.sendExit(CPU_CHANNEL);
	console.sendExit(COMMS_CHANNEL);
	console.sendExit(DISPLAY_CHANNEL);

	console.join();
	cpu.join();
	comms.join();
	radar.join();
	display.join();

	cout << "***** APPLICATION END *****" << endl;
	return EXIT_SUCCESS;
}

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

	while (!stop)
	{
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
