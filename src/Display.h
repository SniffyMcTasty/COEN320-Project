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

    friend void* displayThread(void* arg);

private:
    pthread_t thread;
    name_attach_t *attach = NULL;
    int rows, cols;
    int r, c;

    void setupChannel();
    void destroyChannel();

public:
    Display();
    int join();

    void makeBorders();
};
    
