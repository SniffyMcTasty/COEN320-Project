#include "LoadAlgo.h"

LoadAlgo::LoadAlgo() {
	this->buffer = "";
}

string LoadAlgo::getBuffer() {
	return this->buffer;
}

void LoadAlgo::createLoad(Load load) {
	int nbr_planes;

	if(load == low){
		nbr_planes = rand() % 25 + 25; // ranges from 25 to 49
	} else if(load == medium) {
		nbr_planes = rand() % 25 + 50; // ranges from 50 to 74
	} else if(load == high) {
		nbr_planes = rand() % 25 + 75; // ranges from 75 to 99
	}

	cout << "Creation of load with " << nbr_planes << " planes" << endl;

	this->buffer = "Initial load (Time, ID, X, Y, Z, SpeedX, SpeedY, SpeedZ):\n";

	// creates a plane at t = 0 and has a 1/4 chance to do so afterward at every t
	int t = 0;
	set<int> ids;
	std::pair<std::set<int>::iterator, bool> ret;
	for(int i = 0; i < nbr_planes; t++) {
		if(t == 0 || rand() % 4 == 0) {
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
