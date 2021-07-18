/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to gcode parsing.
*/

#ifndef NCPARSER_H
#define NCPARSER_H

#include <cstring>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

//vars
extern unsigned int speedFastXY;

//struct
struct ncFlagsStruc {unsigned int coord = 90; unsigned int circular = 90; unsigned int workplane = 17; unsigned int compensation = 40; unsigned int unit = 21;};
struct ncLineStruc {int g = -1; int tool = -1; unsigned int f = speedFastXY; double x = 0.; double y = 0.; double z = 0.; double i = 0.; double j = 0.; double k = 0.; double q = 0.; double r = 0.; double startAngle = 0.; double endAngle = 0.; double startAngle1 = 0.; double endAngle1 = 0.; double radius = 0.;};
struct ncToolStruc {int num = -1; float diameter = 0.; float radius = 0.; float length = 0.; float angle = 0.;}; //nc tools struct
struct ncDistTimeStruc {char name [64] = {}; double distWork = 0; double distFast = 0; double distCircular = 0; double distDrill = 0; double timeWork = 0; double timeFast = 0; double timeCircular = 0; double timeDrill = 0;}; //nc extracted lines data struct
struct ncLimitStruc {double xMin = 0; double xMax = 0; double yMin = 0; double yMax = 0; double zMin = 0; double zMax = 0; double zMinWork = 0; double zMaxWork = 0;}; //nc limits struct
struct ncLinesCountStruc {unsigned int all = 0; unsigned int commented = 0; unsigned int skip = 0; unsigned int g0 = 0; unsigned int g1 = 0; unsigned int g2 = 0; unsigned int g3 = 0; unsigned int g81 = 0; unsigned int tools = 0; double totalTimeFast = .0; double totalTimeWork = .0;}; //nc lines struct
struct ncArraySize {unsigned lineStrucLimit = 1; unsigned distTimeStrucLimit = 100; unsigned toolStrucLimit = 100;};

//func
bool inArrayInt (int*, int, int); //check if int in array
void secToClock (char*, double); //convert seconds to char array in format : MM:SS / H:MM:SS
unsigned int NCcountLines (char*); //count number of lines into a file
double numDiffDouble (double, double); //absolute delta between 2 doubles
double rad2deg (double); //convert radian to degree
double deg2rad (double); //convert degree to radian
double angle3points(double, double, double, double, double, double); //compute angle based on start/end/center coordonates of a arc
void arcLimits(double*, double, double, double, double, double, double, int); //compute arc limits
int NCparseFile (char*, ncFlagsStruc*, ncLineStruc*, ncToolStruc*, ncDistTimeStruc*, ncLimitStruc*, ncLinesCountStruc*, ncArraySize*, bool); //extract data from gcode file



#endif