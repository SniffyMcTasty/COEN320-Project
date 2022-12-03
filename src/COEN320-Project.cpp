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
#include "Radar.h"
#include "common.h"

using namespace std;

bool createInputFile();
vector<pair<int, PlaneInfo_t>> readInputFile();

vector<PlaneInfo_t> sendRadarCommand(name_attach_t* attach);
void sendExit(const char* channel);

int main() {
    cout << "***** APPLICATION START *****" << endl;

	srand((int) time(0));
	if( !createInputFile() ) return EXIT_FAILURE; // file directory

	vector<pair<int, PlaneInfo_t>> planeArrivals = readInputFile();

	cout << planeArrivals.size() << " planes read from file" << endl;
	for (size_t i = 0; i < planeArrivals.size(); i++) {
		cout << "t = " << to_string(planeArrivals[i].first) << " -> " << planeArrivals[i].second << endl;
	}

	// create a listener channel to get radar replies (WILL BE IN CENTRAL COMPUTER CONSOLE THREAD EVENTUALLY)
    name_attach_t *attach = NULL;
    if ((attach = name_attach(NULL, MAIN_CHANNEL, 0)) == NULL) {
    	cout << "ERROR: CREATING MAIN SERVER" << endl;
    	return EXIT_FAILURE;
    }

    // airspace tracks all planes
	vector<Plane*> airspace;
	Radar radar(&airspace); // radar thread starter with reference to airspace
	int time = 0;

	// wait for Radar setup to complete (cant ping planes before radar is setup)
	while (!radar.setup);

	// flag tracks if last Radar call returned any planes
    bool hasPlanes = false;

    // keep looping while there is still planes yet to arrive,
    // or once all planes have arrive, keep looping until no more planes detected by radar
	while(!planeArrivals.empty() || hasPlanes) {

		// if there is still planes left to arrive
		if (!planeArrivals.empty()) {
			// if the arrival time of the next plane is now
			if (time >= planeArrivals.front().first) {
				airspace.push_back(new Plane(planeArrivals.front().second)); // get next PlaneInfo from arrivals and create Plane thread
				planeArrivals.erase(planeArrivals.begin());	// delete info from arrivals after Plane started
			}
		}

		// if is true every 5 seconds, use radar to detect planes
		if (!(time % 5)) {
			hasPlanes = false; // default no planes
			for (PlaneInfo_t i : sendRadarCommand(attach)) {
				cout << i << endl; // just print planes for now (MUST GO TO DATA DISPLAY LATER)
				hasPlanes = true; // set to true when at least one plane
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

	sendExit(RADAR_CHANNEL);
	radar.join();

	cout << "***** APPLICATION END *****" << endl;
	return EXIT_SUCCESS;
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

vector<PlaneInfo_t> sendRadarCommand(name_attach_t *attach)
{
	vector<PlaneInfo_t> planes; // return vector
	int coid = 0, replyCnt = 0; // connection ID and cnt for number of expected radar replies
	Msg msg;					// message for IPC

	// open channel to Radar thread
	if ((coid = name_open(RADAR_CHANNEL, 0)) == -1)
	{
		cout << "ERROR: CREATING CLIENT TO RADAR" << endl;
		return planes;
	}

	// create message for Radar Request
	msg.hdr.type = MsgType::RADAR;
	msg.hdr.subtype = MsgSubtype::REQ;
	memset(&msg.info, 0, sizeof(msg.info)); // clear data in PlaneInfo

	// send the message, blocks and waits for reply with return message of replyCnt
	if (MsgSend(coid, (void *)&msg, sizeof(msg), (void *)&replyCnt, sizeof(replyCnt)) < 0)
		cout << "ERROR: PING REQUEST" << endl;

	// print number of plane replies
	//	cout << "RADAR REQ: replies=" << replyCnt << endl;

	// loop for the amount of expected replies
	for (int i = 0; i < replyCnt;)
	{
		// block and wait for a message from RADAR thread (hopefully)
		int rcvid = MsgReceive(attach->chid, (void *)&msg, sizeof(msg), NULL);

		// IDK, WE MAY NEED THESE LINES EVENTUALLY WHEN THIS IS MOVED TO CENTRAL COMPUTER SYSTEM
		//		if (!rcvid && (msg.hdr.code == _PULSE_CODE_DISCONNECT))	{ ConnectDetach(msg.hdr.scoid);		continue; }
		//		if (msg.hdr.type == _IO_CONNECT) 						{ MsgReply(rcvid, EOK, NULL, 0);	continue; }
		//		if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX)	{ MsgError(rcvid, ENOSYS);			continue; }

		// if the received message is a Radar Reply
		if ((msg.hdr.type == MsgType::RADAR) && (msg.hdr.subtype == MsgSubtype::REPLY))
		{
			planes.push_back(msg.info); // add plane info to vector
			MsgReply(rcvid, EOK, 0, 0); // acknowledge the message
			i++;						// increment in this IF statement, only when a plane info has been receive and added to return vector
		}
	}

	name_close(coid); // close the channel
	return planes;	  // return the planes info
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
	fd = creat(FILENAME, S_IRUSR | S_IWUSR | S_IXUSR);

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
	int fd = open(FILENAME, O_RDONLY); // file directory

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
