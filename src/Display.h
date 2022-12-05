/*
	Display.h
	Authors:
		Alexandre Vallières (40157223 – alexandre.vallieres@mail.concordia.ca)
		Samson Kaller (40136815 – samson.kaller@gmail.com)
		Adnan Saab (40075504 – adnan.9821@gmail.com)
		Mohammed Al-Taie (40097284 – altaiem888@gmail.com)
	Description:
		Display System Thread class header.
 */
#pragma once

#include "common.h"

// macros for cursor row, col, positions with Ncurses

#define TITLE_ROW 4
#define COL_START 13
#define ALERT_ROW_START 6
#define RADAR_ROW_START 6
#define RADAR_ROWS_TOTAL 34
#define TITLES "  ID       X       Y      Z     dX     dY   dZ   FL"

class Display {

	// thread function as friend to share private attributes
    friend void* displayThread(void* arg);

private:
    pthread_t thread;	// thread type
    name_attach_t *attach = NULL;	// channel attach type
    int maxRows, maxCols;	//
    int r, c;

    void setupChannel();	// setup IPC channel
    void destroyChannel();	// destroy IPC channel

public:
    Display();	// constructor
    int join();	// join for main()

    // visual border on screen and titles/infos
    void makeBorders();
};
    
