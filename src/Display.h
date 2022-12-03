#pragma once

#include "common.h"
#include "Constants.h"


class Display {

    friend void* displayThread(void* arg);

    private:
    pthread_t thread;
    name_attach_t *attach = NULL;

    void setupChannel();
    void destroyChannel();

    public:
    Display();
    int join();
};
    
