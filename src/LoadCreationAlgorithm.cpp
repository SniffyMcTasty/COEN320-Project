# include "LoadCreationAlgorithm.h"

LoadCreationAlgorithm::LoadCreationAlgorithm() {
	this->buffer = "";
}

string LoadCreationAlgorithm::getBuffer() {
	return this->buffer;
}

void LoadCreationAlgorithm::createLoad() {

	this->buffer = "This is a test";
	this->buffer += "\nThis is supposed to be added in another line";

}
