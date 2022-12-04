#include "Console.h"

void* consoleThread(void* arg) {

	Console& console = *((Console*)arg);

	cout << "Running Console Thread" << endl;

//	int rows, cols;
//	getmaxyx(stdscr, rows, cols);

	string buffer;
	while (!console.exit) {

		delay(100);

		pthread_mutex_lock(&mtx);
		unsigned char c = getch();
		pthread_mutex_unlock(&mtx);

		if (c == ERR)
			continue;

		if (c == '\n') {
            int x, y;

            pthread_mutex_lock(&mtx);
            getyx(stdscr, y, x);
            move(y, x - buffer.size());
            for (size_t i = 0; i < buffer.size(); i++)
            	printw(" ");
            pthread_mutex_unlock(&mtx);

			buffer = "";
		}
		else if (c == 0x7F) {
			size_t size = buffer.size();
			if (size > 0) {
				int y, x;

				pthread_mutex_lock(&mtx);
				getyx(stdscr, y, x);
				mvprintw(y, x - 1, " ");
				move(y, x);
	            pthread_mutex_unlock(&mtx);

				buffer = buffer.substr(0, size - 1);
			}
		}

		if ((c == ' ') || (c == '-') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
			buffer += c;

		pthread_mutex_lock(&mtx);
		mvprintw(52, 4, "%s", buffer.c_str());
		refresh();
		pthread_mutex_unlock(&mtx);
	}
	endwin();
	cout << "Exit Console Thread" << endl;
	pthread_exit(NULL);
}

Console::Console() {
	if (pthread_create(&thread, NULL, consoleThread, (void *)this))
		cout << "ERROR: MAKING PLANE THREAD" << endl;
}

int Console::join() {
	int r, c;
	string end = "Simulation over. Hit any key to exit.";

	pthread_mutex_lock(&mtx);
	getmaxyx(stdscr, r, c);
	mvprintw(r/2, (c - end.size())/2, end.c_str());
	while(getch() == ERR);
    pthread_mutex_unlock(&mtx);

    exit = true;
	return pthread_join(thread, NULL);
}

