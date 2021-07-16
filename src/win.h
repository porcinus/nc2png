/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Windows specific.
*/

#ifndef WIN_H
#define WIN_H

#include <stdio.h> //stream io
#include <windows.h>

//funct
bool CtrlHandler(DWORD); //handle ctrl-c on windows to avoid ansicon glitch that doesn't reset ansi code





//vars
extern bool debug; //debug mode bool




#endif