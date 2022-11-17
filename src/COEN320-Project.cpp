#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <cstring>

#include "LoadCreationAlgorithm.h"

using namespace std;

int main() {

	int fd; // file directory
	long unsigned int sw;  // size written
	LoadCreationAlgorithm algo;

	// open file with read, write, execute permissions and replace if existing
	fd = creat( "/data/home/qnxuser/loadInput.txt", S_IRUSR | S_IWUSR | S_IXUSR );

	algo.createLoad();

	// write buffer to file
	string buffer = algo.getBuffer();
	char char_buffer[buffer.length()+1];
	strcpy(char_buffer, buffer.c_str());
	sw = write( fd, char_buffer, sizeof( char_buffer ) );

	// test for error
	if( sw != sizeof( char_buffer ) ) {
		perror( "Error writing loadInput.txt" );
		return EXIT_FAILURE;
	}
	cout << "Input load file created successfully" << endl;

	// close the file
	close( fd );

	return EXIT_SUCCESS;
}
