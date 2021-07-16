/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to Ansicon, used on Windows to provide console ansi color compatibility.
*/

#ifndef ANSICON_H
#define ANSICON_H

#include <stdio.h> //stream io
//#include <cstdlib> //c standard
//#include <cstring> //string
#include <limits.h> //limits
#if defined _WIN32 || defined __CYGWIN__
    #include <windows.h>
    //#include <tchar.h>
    #include <psapi.h>
#endif
#include <sys/stat.h> //file stat

//func
int checkAnsiconExists (void);
int checkAnsiconModule (void);

//vars
extern bool debug; //debug mode bool
extern bool shouldRunAnsicon; //used to avoid run loop if ansicon not installed in windows

#endif