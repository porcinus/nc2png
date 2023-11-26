/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to multilanguage support.

upd 0.4a ok
*/

#ifndef LANG_H
#define LANG_H

#include <stdio.h>
#include <cstring>
#if defined _WIN32 || defined __CYGWIN__
    #include <windows.h>
#else
    #include <clocale>
#endif
#include "debug.h"

//enum
#define LANG_COUNT 2
enum LANG_ID {LANG_EN, LANG_FR};

//funct
unsigned int getLocale(void); //get system locale id

//vars
extern unsigned int language; //language id




#endif