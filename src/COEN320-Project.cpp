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
#include "Plane.h"
#include "common.h"

using namespace std;

#define FILENAME "/data/home/qnxuser/loadInput.txt"

bool createInputFile();
vector<pair<int, PlaneInfo_t>> readInputFile();

int main() {

	srand((int) time(0));
	if( !createInputFile() ) return EXIT_FAILURE; // file directory

	vector<pair<int, PlaneInfo_t>> planes = readInputFile();

	cout << planes.size() << " planes read from file" << endl;
	for (int i = 0; i < planes.size(); i++) {
		cout << "t = " << to_string(planes.at(i).first) << " -> " << planes.at(i).second << endl;
	}

	vector<Plane*> airspace;
	int time = 0;

	// create threads from while loop with 1s counter (initialized at 0)
	while(true) {
		// check if there is a plane coming at current time
		// -> if so, create thread from plane, add it to airspace vector
		//  and then do this: planes.erase(planes.begin());
		//
		// -> do nothing if it's not the case

		if(planes.empty()) break;

		// delay of 1s
		delay(1000);

		time++;
	}

	// join every thread

	return EXIT_SUCCESS;
}

/*#include "common.h"
#include "Plane.h"

int main(int argc, char *argv[])
{
    srand(time(NULL));
    cout << "***** APPLICATION START *****" << endl;

    Plane plane(Plane::randomInfo());
    while (plane.inZone())
    {
        delay((rand() % 20 + 1) * 100);
        plane.ping();
    }
    plane.join();

    cout << "***** APPLICATION END *****" << endl;
    return EXIT_SUCCESS;
}*/

bool createInputFile() {
	int fd; // file directory
	long unsigned int sw;  // size written
	LoadCreationAlgorithm algo;

	// open file with read, write, execute permissions and replace if existing
	fd = creat(FILENAME, S_IRUSR | S_IWUSR | S_IXUSR );

	// prompt user for load
	string input;
	bool stop = false;

	while(!stop) {
		cout << "Enter desired load (low, medium, high): ";
		cin >> input;
		if(input.compare("low") == 0) {
			algo.createLoad(low);
		} else if (input.compare("medium") == 0) {
			algo.createLoad(medium);
		} else if (input.compare("high") == 0) {
			algo.createLoad(high);
		} else {
			cout << "Invalid input, try again." << endl;
			continue;
		}
		stop = true;
	}

	// write buffer to file
	string buffer = algo.getBuffer();
	char char_buffer[buffer.length()+1];
	strcpy(char_buffer, buffer.c_str());
	sw = write( fd, char_buffer, sizeof( char_buffer ) );

	// test for error
	if( sw != sizeof( char_buffer ) ) {
		perror( "Error writing loadInput.txt" );
		return false;
	}
	cout << "Input load file created successfully" << endl;
	close( fd );

	return true;
}

vector<pair<int, PlaneInfo_t>> readInputFile() {
	int sr;  // size read
	int fd = open(FILENAME, O_RDONLY); // file directory

	// read the file
	char buffer[100];
	string content;

	sr = read(fd, buffer, sizeof(buffer));
	content += buffer;
	while (sr == 100) {
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
	while( getline(ss, line) ) {
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

