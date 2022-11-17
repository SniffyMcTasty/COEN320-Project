#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <cstring>

#include "Constants.h"

using namespace std;

class LoadCreationAlgorithm {
	private:
		string buffer;

	public:
		LoadCreationAlgorithm();
		string getBuffer();
		void createLoad(Load load);
};
