#ifndef LOAD_CREATION_ALGORITHM_H
#define LOAD_CREATION_ALGORITHM_H

#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include <set>

#include "Constants.h"
#include "InitialPlane.h"

using namespace std;

class LoadCreationAlgorithm {
	private:
		string buffer;

	public:
		LoadCreationAlgorithm();
		string getBuffer();
		void createLoad(Load load);
};

#endif
