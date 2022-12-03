#ifndef CONSTANTS_H
#define CONSTANTS_H

/***** PLANE CREATION *****/

#define BOTTOM 15000
#define TOP 40000
#define HEIGHT 25000
#define WIDTH 100000
#define MIN_SPEED 733 // typical min speed for aircrafts is 500 mph (~733 ft/s)
#define MAX_SPEED 1026 // typical max speed for aircrafts is 700 mph (~1026 ft/s)
#define SPEED_INTERVAL 293 // difference between min and max speed
#define ID_MIN 1000
#define ID_INTERVAL 9000

#define INPUT_FILENAME "/data/home/qnxuser/loadInput.txt"
#define OUTPUT_FILENAME "/data/home/qnxuser/history.txt"

//#define MAIN_CHANNEL "mainChannel"
#define RADAR_CHANNEL	"radarChannel"
#define CPU_CHANNEL		"cpuChannel"
#define DISPLAY_CHANNEL	"displayChannel"

enum Load {low, medium, high};

#endif
