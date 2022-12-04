#pragma once

#include <set>
#include "common.h"

#include "Plane.h"

using namespace std;

class LoadAlgo {
	private:
		string buffer;

	public:
		LoadAlgo();
		string getBuffer();
		void createLoad(Load load);
};
