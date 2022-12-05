/*
	LoadAlgo.cpp
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Class that generates and input file for low, medium, high, or
		overload of planes in airspace.
 */
#include "LoadAlgo.h"

// Constructor
LoadAlgo::LoadAlgo() {
	this->buffer = "";
}

// get the string buffer
string LoadAlgo::getBuffer() {
	return this->buffer;
}

// create the desired load
void LoadAlgo::createLoad(Load load) {
	int nbr_planes, interval;

	// explicitly setting number of Aircrafts and IO traffic by load
	if(load == low){
		nbr_planes = randRange(25, 75); // ranges from 20 to 40
		interval = 4;
	} else if(load == medium) {
		nbr_planes = randRange(76, 125); // ranges from 40 to 60
		interval = 3;
	} else if(load == high) {
		nbr_planes = randRange(126, 175); // ranges from 60 to 79
		interval = 2;
	} else if (load == overload) {
		nbr_planes = randRange(176, 225); // ranges from 80 to 100
		interval = 1;
	}

	cout << "Creation of load with " << nbr_planes << " planes" << endl;

	this->buffer = "Initial load (Time, ID, X, Y, Z, SpeedX, SpeedY, SpeedZ):\n";

	// creates a plane at t = 0 and has a 1/4 chance to do so afterward at every t
	int t = 0;
	set<int> ids;
	std::pair<std::set<int>::iterator, bool> ret;
	for(int i = 0; i < nbr_planes; t++) {
		if(t == 0 || rand() % interval == 0) {
			i++;
			// create id
			int id = rand() % ID_INTERVAL + ID_MIN; // ranges from 1000 to 9999
			ret = ids.insert(id);
			// check if id already exists
			while(ret.second == false) {
				id = rand() % ID_INTERVAL + ID_MIN; // ranges from 1000 to 9999
				ret = ids.insert(id);
			}
			Plane plane(t, id);
			this->buffer += plane.format() + "\n";
		}
	}
}
