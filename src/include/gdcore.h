/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to libGD, generate preview from a custom structs (ncparser.h/cpp).

Important note: functions bypassing LibGD function never uses gdImageBoundsSafe().

upd 0.4a ok
*/

#ifndef GDCORE_H
#define GDCORE_H

#include <stdio.h>
#include <cstdlib>
#include <math.h>
#include <limits.h>
extern "C" {
#include "gd.h"
#include <gdfonts.h>
#include <gdfontg.h>
#include <zlib.h>
}
#include "debug.h"
#include "ncparser.h"
#include "gdcore_depth.h"
#include <errno.h>

//funct
int NNSclampInt(int num, int min, int max); //clamp int value
float NNSclampFloat(float num, float min, float max); //clamp float value
int NNSintFade(int low, int high, float fade); //proportional int gradian, fade:0.0 to 1.0
float NNSfloatFade(float low, float high, float fade); //proportional float gradian, fade:0.0 to 1.0
int NNSintFadeNoClamp(int low, int high, float fade); //proportional int gradian, fade:0.0 to 1.0, no clamping
float NNSfloatFadeNoClamp(float low, float high, float fade); //proportional float gradian, fade:0.0 to 1.0, no clamping
void NNSintArrFade(int* arrDest, int* arr1, int* arr2, int size, float fade); //proportional int array gradian
int NNSgdImageColorFade(int* rgbArr1, int* rgbArr2, float fade); //proportional color gradian
void NNSgdImageDepth2Color(gdImagePtr image, int* rgbArrLow, int* rgbArrHigh); //convert gdImage from 0-255 colors (highest-lowest) to given arrays (r,g,b), alpha will be 0 or 127
void NNSgdImageFill(gdImagePtr image, int color); //gdImageFill() replacement to avoid transparent color glich
void NNSgdImageTriangleFilled(gdImagePtr image, int x1, int y1, int x2, int y2, int x3, int y3, int color); //draw filled triangle, based on Adafruit GFX library fillTriangle()
void NNSgdImagePolygonFilled(gdImagePtr image, gdPointPtr pointArr, int pointCount, int color); //draw filled polygon, bypass LibGD complex function
void NNSgdImageLineDashed(gdImagePtr image, int x1, int y1, int x2, int y2, int color1, int color2, int space); //draw dashed line the proper way, GD one is glitched
void NNSgdImageArc(gdImagePtr image, float cx, float cy, float width, float height, float start, float end, int color, int resolution); //draw arc, allow precision definition, GD one draw one line per degree
void NNSgdImageArrow(gdImagePtr image, float x, float y, float lenght, float width, float height, int dir, int color, bool filled); //draw simple arrow
void sec2charArr(char* arr, double time); //convert seconds to char array in format : XXsec / XXmin / XXh XXmin
int NNSPNGaddPPM(char* file, unsigned int ppm); //write specific PPM to PNG file, can be done via libpng but it is faster that way
int gdPreview(char* file, int gdPrevWidth, int gdPrevArcRes, bool exportDepthMap, ncLineStruct* lines, ncToolStruct* tools, ncCommentStruct* comments, ncLimitStruct* limits, ncSummaryStruct* summary, ncArraySize* arrSizes, bool debugOutput); //generate image preview from ncparser data

//gdcore_depth.h
extern int NNSclampIntPermissive(int num, int min, int max, int limit); //clamp int value withing range, if value over/under 'limit', value is unchanged
extern long bwPixelDrawn;

//ncparser.h
extern double deg2rad(double deg); //convert degree to radian

//nc2png.h
extern bool userInput(bool alwaysTrue); //here for debug, read stdin, return true when return char detected, false overwise

//vars
extern bool debug; //debug mode bool
extern unsigned int speedFastXY; //max XY moving speed
extern unsigned int speedFastZ;  //max Z moving speed
extern unsigned int speedPercent; //max running percent speed, used as a override
extern unsigned int gdWidth; //max grid width/height in px
extern unsigned int gdArcRes; //arc drawing resolution
extern unsigned int language; //language id
extern char programversion[]; //program version



#endif