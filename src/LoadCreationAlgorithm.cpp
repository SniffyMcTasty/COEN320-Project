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

	} else if(load == medium) {

	} else if(load == high) {

	}

	this->buffer = "This is a test";
	this->buffer += "\nThis is supposed to be added in another line";

}
