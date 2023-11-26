/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Specific to Linux.

upd 0.4a ok
*/

#ifndef NC2PNG_LINUX_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <sys/stat.h>
#include "debug.h"


#if (defined _WIN32) //used for windows debug
    char *ptsname(int fildes){fildes++; return NULL;} //placeholder
#endif

//func
bool runningFromTerminal(void); //try to detect if program running from terminal
const char* terminalFilePath(void); //try to get proper terminal executable file

//vars
#define terminalArrCount 11
const char* terminalCmd[terminalArrCount] = { //terminal full paths WITH execute command, based on https://en.wikipedia.org/wiki/List_of_terminal_emulators
    "/usr/bin/gnome-terminal -e", //gnome
    "/usr/bin/konsole -e", //kde
    "/usr/bin/xfce4-terminal -e", //lxce
    "/usr/bin/lxterminal -e", //lxde
    "/etc/alternatives/x-terminal-emulator -e", //Ubuntu auto configurated terminal
    "/usr/bin/terminator -e",
    "rxvt-unicode -e",
    "/usr/bin/kitty -e",
    "/usr/bin/tilix -e",
    "/usr/bin/tilda -c",
    "/usr/bin/xterm -e", //x11 fallback
};

#endif