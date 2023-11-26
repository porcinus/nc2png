/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to gcode parsing.

update 0.4a:
   - ncCountLines(), ncParseFile(), ncParseReportTerm() massive rework.
   - switched structures double to float for memory usage.

*/

#ifndef NCPARSER_H
#define NCPARSER_H

#define _USE_MATH_DEFINES //allow constants definition from math.h
#define NCPARSER_NAME_LEN 129 //gcode comment name array length
#define NCPARSER_DETECT_MIN 0.0001 //minimum distance allowed to compute travel/time
#define NCPARSER_COMMENT_ARRAY 100 //limit comment section to this amount

#include <new>
#include <cstring>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "debug.h"
#include <errno.h>

//struct
struct ncFlagsStruct { //store gcode flags
    unsigned int coord = 90; //G90:absolute, G91:relative
    unsigned int circular = 90; //G90.1:absolute, G91.1:relative
    unsigned int workplane = 17; //G17:XY, G18:ZX, G19:YZ
    unsigned int compensation = 40; //G40:none, G41:left, G42:right
    unsigned int unit = 21; //G20:inch, G21:mm
};
struct ncLineStruct { //store gcode line data
    unsigned int line = 0; //line in file
    int g = -1; //function
    int tool = -1; //tool number
    unsigned int comment = 0; //comment id, used to split the program into sections
    unsigned int f = 100; //speed in mm/min
    float x = 0., y = 0., z = 0.; //linear position
    float i = 0., j = 0., k = 0.; //circular data
    float q = 0., r = 0.; unsigned int retractMode = 0; //drilling data
    float startAngle = 0., endAngle = 0., radius = 0.; //computed angles for drawing
    float startAngle1 = 0., endAngle1 = 0.; //computed angles for drawing, arc pass rad 0
};
struct ncToolStruct {int num = -1; float diameter = 0.; float radius = 0.; float length = 0.; float angle = 0.;}; //store tools data
struct ncCommentStruct { //gcode file sections, time in min
    char name[NCPARSER_NAME_LEN] = {'\0'};
    float distTotal = 0, timeTotal = 0;
    float distFast = 0, timeFast = 0;
    float distWork = 0, timeWork = 0;
    float distCircular = 0, timeCircular = 0;
    float distDrill = 0, timeDrill = 0;
};

struct ncLimitStruct { //boundaries
    bool blockDetected = false;
    float xMin = 0, xMax = 0;
    float yMin = 0, yMax = 0;
    float zMin = 0, zMax = 0;
    float zMinWork = 0, zMaxWork = 0;
    float toolMax = 0; //biggest tool detected
};
struct ncSummaryStruct { //summary of the gcode file, lines/times/distances, time in min
    unsigned int all = 0, commented = 0, skip = 0;
    unsigned int g0 = 0, g1 = 0;
    unsigned int g2 = 0, g3 = 0;
    unsigned int g81 = 0;
    unsigned int tools = 0;
    float distTotal = 0, timeTotal = 0;
    float distFastTotal = 0, timeFastTotal = 0;
    float distWorkTotal = 0, timeWorkTotal = 0;
    float distCircularTotal = 0, timeCircularTotal = 0;
    float distDrillTotal = 0, timeDrillTotal = 0;
};
struct ncArraySize { //arrays boundaries
    unsigned int lineStrucLimit = 1;
    unsigned int commentStrucLimit = 1;
    unsigned int toolStrucLimit = 1;
};

//func
bool inArrayInt(int* arr, int value, int arrSize); //check if int in array
char *strtok_chr(char* str, const char delimiter); //custom implement of strtok with char delimiter
void charArrFill(char* arr, unsigned int length, char chr); //fill char array with given char, array will be null terminated 
void secToClock(char* arr, double value); //convert seconds to char array in format : MM:SS / H:MM:SS
double numDiffDouble(double n1, double n2); //absolute delta between 2 doubles
float numDiffFloat(float n1, float n2); //absolute delta between 2 floats
double rad2deg(double rad); //convert radian to degree
double deg2rad(double deg); //convert degree to radian
double angle3points(double centerX, double centerY, double x1, double y1, double x2, double y2); //compute angle based on start/end/center coordonates of a arc
void arcLimits(double* limitArr, double centerX, double centerY, double width, double height, double angleStart, double angleEnd, int resolution); //compute arc limits
unsigned int ncCountLines(char* file); //count number of lines into a file, ignore empty or commented lines (except if starts by a space)
int ncParseFile(char* file, unsigned int maxSpeedXY, unsigned int maxSpeedZ, ncFlagsStruct* flags, ncLineStruct* lines, ncToolStruct* tools, ncCommentStruct* comments, ncLimitStruct* limits, ncSummaryStruct* summary, ncArraySize* arrSizes, bool debugOutput); //extract data from gcode file
void ncParseReportTerm(char* filename, unsigned int maxSpeedXY, unsigned int maxSpeedZ, unsigned int speedPercent, ncFlagsStruct* flags, ncToolStruct* tools, ncCommentStruct* comments, ncLimitStruct* limits, ncSummaryStruct* summary, ncArraySize* arrSizes); //report gcode infos to terminal

//extern func
extern void sec2charArr(char* arr, double time); //gdcode.h: convert seconds to char array in format : XXsec / XXmin / XXh XXmin
extern void UTF8toCP850(char* dest, char* source); //ansicon.h: convert UTF8 chars to CP850 for proper display
extern void printfTerm(const char* format, ...); //ansicon.h: printf clone to do UTF8 to CP850 on the fly, based on printf source code

//extern vars
extern unsigned int streamBufferSize; //nc2png.h: try to optimize file writing using bigger buffer

#endif