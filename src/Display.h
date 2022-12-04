#pragma once

#include "common.h"

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
    
