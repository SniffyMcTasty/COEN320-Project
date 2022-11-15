#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include "Constants.h"
#include "LoadCreationAlgorithm.h"

using namespace std;

int main() {

	srand((int) time(0));
	int fd; // file directory
	long unsigned int sw;  // size written
	LoadCreationAlgorithm algo;

	// open file with read, write, execute permissions and replace if existing
	fd = creat( "/data/home/qnxuser/loadInput.txt", S_IRUSR | S_IWUSR | S_IXUSR );

	// prompt user for load
	string input;
	bool stop = false;

	while(!stop) {
		cout << "Enter desired load (low, medium, high): ";
		cin >> input;
		if(input.compare("low") == 0) {
			algo.createLoad(low);
		} else if (input.compare("medium") == 0) {
			algo.createLoad(medium);
		} else if (input.compare("high") == 0) {
			algo.createLoad(high);
		} else {
			cout << "Invalid input, try again." << endl;
			continue;
		}
		stop = true;
	}

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

// #include <pthread.h>
// #include <signal.h>
// #include <sys/dispatch.h>
// using namespace std;
// #define CHANNEL "planeChannel"
// enum MsgType : _Uint16t
// {
// 	TIMEOUT,
// 	MSG
// };
// struct Msg
// {
// 	_pulse hdr;
// 	string data;
// 	char plane;
// };
// int connecId;
// int sendMsg()
// {
// 	Msg msg;
// 	msg.hdr.type = MsgType::MSG;
// 	msg.data = "hello form the main";
// 	return MsgSend(connecId, &msg, sizeof(msg), NULL, 0);
// }

// void *threadFunction(void *arg)
// {
// 	// int &intRef = *((int *)arg);
// 	// int *intPtr = (int *)arg;
// 	// int intVar = *((int *)arg);

// 	// for (size_t i = 0; i < 5; i++)
// 	// {
// 	// 	delay(intVar);
// 	// 	// delay(int1);
// 	// 	// delay(int2);
// 	// 	cout << "hello " << i << endl;
// 	// }
// 	// // function that has  void pointer and
// 	// cout << "thread done" << endl;
// 	name_attach_t *attach;

// 	timer_t timerId;
// 	// channel to listen
// 	{
// 		if ((attach = name_attach(NULL, CHANNEL, 0)) == NULL)
// 			pthread_exit(NULL);
// 		// channel to send
// 		if ((connecId = name_open(CHANNEL, 0)) == -1)
// 			pthread_exit(NULL);
// 	}
// 	///////////////////////timer setup
// 	{
// 		sigevent event;
// 		event.sigev_notify = SIGEV_PULSE;
// 		event.sigev_coid = connecId;
// 		event.sigev_code = MsgType::TIMEOUT;
// 		timer_create(CLOCK_MONOTONIC, &event, &timerId);

// 		itimerspec timerSpec{
// 			1, // it_Value.sec
// 			0, // it_value.nsec
// 			1, // it_interval.sec
// 			0  // it_interval.nsec
// 		};
// 		timer_settime(timerId, 0, &timerSpec, NULL);
// 	}
// 	//////thread running
// 	cout << "thread Ready staring " << endl;
// 	int cout = 0;
// 	while (true)
// 	{
// 		int recId;
// 		Msg msg;
// 		recId = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
// 		if (recId == -1)
// 			break;
// 		if ((recId == 0) && (msg.hdr.code == _PULSE_CODE_DISCONNECT))
// 		{
// 			ConnectDetach(msg.hdr.scoid);
// 			continue;
// 		}
// 		if (msg.hdr.type == _IO_CONNECT)
// 		{
// 			MsgReply(recId, EOK, NULL, 0);
// 			continue;
// 		}
// 		if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX)
// 		{
// 			MsgError(recId, ENOSYS);
// 			continue;
// 		}
// 		// the messge ment to us
// 		if (msg.hdr.type == MsgType::TIMEOUT)
// 		{
// 			std::cout << "helo frome timer " << std::endl;
// 			if (++cout > 5)
// 				break;
// 		}
// 		if (msg.hdr.type == MsgType::MSG)
// 		{
// 			std::cout << msg.data << std::endl;
// 		}
// 		MsgReply(recId,EOK, 0, 0);
// 	}
// 	////////destroy
// 	itimerspec off{0, 0, 0, 0};
// 	timer_settime(timerId, 0, &off, NULL);
// 	timer_delete(timerId);
// 	name_close(connecId);
// 	name_detach(attach, 0);

// 	pthread_exit(NULL);
// }

// int main(int argc, char *argv[])
// {
// 	//	std::cout << "##############Appliction start######### " << std::endl;
// 	//	int returnCode;
// 	//
// 	//	pthread_t threadPlane; // thread that has type pthread ID
// 	//	// int timeOut;
// 	//	// int timeOut_ms = 1000;
// 	//	// std::cout << "please enter the valuer timeout sec" << std::endl;
// 	//	// cin >> timeOut;
// 	//	// timeOut *= 1000;
// 	//	int *ptr = new int;
// 	//	*ptr = 100;
// 	//	std::cout << "thread created fsdfscreating " << std::endl;
// 	//	if (returnCode = pthread_create(&threadPlane, NULL, threadFunction, (void *)ptr)) // inside the () it is a cast(void* )
// 	//	{
// 	//		std::cout << "error during pthread_create " << std::endl;
// 	//		// we have to
// 	//		return EXIT_FAILURE;
// 	//	}
// 	//	std::cout << "wating for join" << std::endl;
// 	//	pthread_join(threadPlane, NULL);
// 	//
// 	cout << "##############Appliction start######### " << endl;
// 	pthread_t myThread;
// 	std::cout << "creating thread" << std::endl;
// 	pthread_create(&myThread, NULL, threadFunction, NULL);
// 	while (true)
// 	{
// 		string s;
// 		cin >> s;
// 		if (s == "m")
// 			sendMsg();
// 		if (s == "x")
// 			break;
// 	}
// 	std::cout << "whaiting for join" << std::endl;

// 	pthread_join(myThread, NULL);
// 	cout << "########app end###########" << endl;
	return EXIT_SUCCESS;
}
