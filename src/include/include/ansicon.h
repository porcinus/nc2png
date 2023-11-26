/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to Ansicon, also used on Windows to provide console ansi color compatibility.

upd 0.4a ok
*/

#ifndef ANSICON_H
#define ANSICON_H

#define PRINTFTERM_BUFFER_LEN 4096

#include <stdio.h> //stream io
#if (!defined WINCON && (defined _WIN32 || defined __CYGWIN__))
    #include <windows.h>
    #include <psapi.h>
#endif

#include <stdarg.h>
#include <sys/stat.h> //file stat
#include <cstring>
#include "debug.h"

//func
bool checkAnsiconExists(void); //search thru PATH for ansicon
bool checkAnsiconModule(void); //search thru current process for ansicon module loaded
void UTF8toCP850(char* dest, char* source); //convert utf8 chars to CP850 for proper display
void printfTerm(const char* format, ...); //printf clone to do UTF8 to CP850 on the fly, based on printf source code

#endif