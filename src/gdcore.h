/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to libGD, generate preview from a custom structs (ncparser.h/cpp).
*/

#ifndef GDCORE_H
#define GDCORE_H

#include <stdio.h>
#include <cstdlib>
extern "C" {
#include "gd.h"
#include <gdfonts.h>
#include <gdfontg.h>
}
#include <math.h>
#include <limits.h>
#include "ncparser.h"

enum STR_GD {TOOL,GRID,AXIS,TOOLPATHS,FAST,LINEAR,CIRCULAR,DURATION,FEED,PNGPATH};

const char *strGDEN [] = {
/*TOOL*/ "Tool",
/*GRID*/ "Grid",
/*AXIS*/ "Axis",
/*TOOLPATHS*/ "Toolpaths",
/*FAST*/ "Fast",
/*LINEAR*/ "Linear Interpolation",
/*CIRCULAR*/ "Circular Interpolation",
/*DURATION*/ "Theorical duration",
/*FEED*/ "Feed",
/*PNGPATH*/ "Preview path"
};

const char *strGDFR [] = {
/*TOOL*/ "Outil",
/*GRID*/ "Grille",
/*AXIS*/ "Axes",
/*TOOLPATHS*/ "Chemin Outils",
/*FAST*/ "Rapide",
/*LINEAR*/ "Interpolation Lineaire",
/*CIRCULAR*/ "Interpolation Circulaire",
/*DURATION*/ "Temps Total Theorique",
/*FEED*/ "Avance",
/*PNGPATH*/ "Chemin de l'apercu"
};

const char **strGD [] = {strGDEN, strGDFR};

//funct
void nns_gd_test (void);
void NNSgdImageLineDashed (gdImagePtr, int, int, int, int, int, int, int);
int NNSgdImageColorAllocateFade (gdImagePtr, int*, int*, double);
void NNSgdImageArcThick (gdImagePtr, int, int, int, int, int, int, int, int, int);
void NNSgdImageLineThick (gdImagePtr, int, int, int, int, int, int);
void NNSgdImageLineThickV (gdImagePtr, int, int, int, int, double, double, int);
void NNSgdImageArcThickV (gdImagePtr, int, int, int, int, int, int, double, double, int, int);
void NNSgdImageArrow (gdImagePtr, int, int, int, int, int, int, int, bool);
void NNSgdImageArc (gdImagePtr, int, int, int, int, int, int, int, int);
void sec2charArr (char*, double);
int gdPreview (char*, int, int, ncFlagsStruc*, ncLineStruc*, ncToolStruc*, ncDistTimeStruc*, ncLimitStruc*, ncLinesCountStruc*, bool);
extern double deg2rad (double);

//vars
extern bool debug; //debug mode bool
extern unsigned int speedFastXY; //max XY moving speed
extern unsigned int speedFastZ;  //max Z moving speed
extern unsigned int speedPercent; //max running percent speed, used as a override
extern unsigned int gdWidth; //max grid width/height in px
extern unsigned int gdArcRes; //arc drawing resolution
extern unsigned int language; //language id
extern char programversion[];



#endif