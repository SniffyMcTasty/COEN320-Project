/*
	LoadAlgo.h
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		LoadAlgo class header file.
 */
#pragma once

#include <set> // set of unique IDs
#include "common.h"
#include "Plane.h"

using namespace std;

class LoadAlgo {
	private:
		string buffer; // string buffer for file print contents

	public:
		LoadAlgo();			// constructor
		string getBuffer();	// return the string with whole input
		void createLoad(Load load); // generate the load
};
