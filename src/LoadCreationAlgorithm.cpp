# include "LoadCreationAlgorithm.h"

LoadCreationAlgorithm::LoadCreationAlgorithm() {
	this->buffer = "";
}

string LoadCreationAlgorithm::getBuffer() {
	return this->buffer;
}

void LoadCreationAlgorithm::createLoad(Load load) {
	int nbr_planes;

	if(load == low){
		nbr_planes = rand() % 25 + 25; // ranges from 25 to 49
	} else if(load == medium) {
		nbr_planes = rand() % 25 + 50; // ranges from 50 to 74
	} else if(load == high) {
		nbr_planes = rand() % 25 + 75; // rnges from 75 to 99
	}

	cout << "Creation of load with " << nbr_planes << " planes" << endl;

	this->buffer = "Initial load (Time, ID, X, Y, Z, SpeedX, SpeedY, SpeedZ):";

	// creates a plane at t = 0 and has a 1/4 chance to do so afterward at every t
	int t = 0;
	for(int i = 0; i < nbr_planes; t++) {
		if(t == 0 || rand() % 4 == 0) {
			i++;
			InitialPlane plane(t);
			this->buffer += "\n" + plane.toString();
		}
	}
}
