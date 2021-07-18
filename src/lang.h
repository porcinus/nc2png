/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to multilanguage support.
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

//enum
enum LANG_ID {LANG_EN, LANG_FR} ;

//funct
unsigned int getLocale (void); //get system locale id

//vars
extern bool debug; //debug mode bool




#endif