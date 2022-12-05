# COEN320-Project

## Cloning the project

To be able to work in your own QNX workspace, clone the repository into the workspace folder next to the folder containing your VM.

![image](https://user-images.githubusercontent.com/68755892/200685346-38ff55c6-d825-425d-979a-dfdc6bf7f3d2.png)

## Compiling the code

Since this project uses the `ncurses` library and it needs to be linked to be linked to the code's objects, make sure to use this repository's Makefile; it contains all the required changes to run the code.

## Running the code

Because of the `ncurses` library, the code needs to be started on the Momentics IDE and without continuing further, started again from the QNX VM. The temporarily built project is located at `/tmp/COEN320-Project` and will stay there only while it is running on the IDE.

## Operator commands

<b>Change position</b>: changePos [ID] [ANGLE]
(actually changes horizontal direction, + -> turn left, - -> turn right, -25 < ANGLE <25)

<b>Change altitude</b>: changeAlt [ID] [ALTITUDE]

<b>Change speed</b>: changeSpeed [ID] [MAGNITUDE]

<b>Get airplane info</b>: info [ID]

<b>Change n parameter</b>: changeWindow [N]
(interval of time for violations)