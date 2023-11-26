/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

NNSbw is meant to handle pseudo greyscale image.
Generate/interact image to bypass memory and performance issues related to large resolution images using libGD.
Stucts, functions (code/behavior), comments are partially based on libGD and Adafruit GFX library.
Please check libraries licenses if you want to redist some parts of this code.
Important notes:
- most functions have limited safety regarding overflows.
- this code isn't meant to handle all possible kind of primitives but only the ones needed for this project.

upd 0.4a ok
*/

#ifndef GDCORE_DEPTH_H
#define GDCORE_DEPTH_H

#include <stdint.h>
#include <cstring>
#include <new>
#include <cmath>
#include <limits.h>
extern "C" {
#include "gd.h"
}
#include "debug.h"
#include <errno.h>

typedef struct NNSbwImageStruct {
    int width, height; //real image width and height
    int *bitmap; //bitmap buffer, <0 || >255 is transparent
    int **bitmapPtr; //bitmap[y][x]
} NNSbwImage;
typedef NNSbwImage *NNSbwImagePtr;

typedef struct {int x, y;} NNSbwPoint, *NNSbwPointPtr;


//math
int NNSclampIntPermissive(int num, int min, int max, int limit); //clamp int value withing range, if value over/under 'limit', value is unchanged

//image buffer related
NNSbwImagePtr NNSbwImageCreate(int width, int height); //allocate image, to be freed with NNSbwImageDestroy(), transparentIndex: -1:none, 0:black, 1:white
void NNSbwImageDestroy(NNSbwImagePtr image); // //deallocate image, image pointer needs to be set to nullptr afterward

//NNSbw to libGD bridge
void NNSbwImage2gdGreyscaleCopy(NNSbwImagePtr image, gdImagePtr gdImage); //copy BW to gd true color image, color < 0 are not be copied, > 255 clamped to 255. Always copy from BW XY0 to GD XY0, overflow is ignored

//drawing
void NNSbwSetPixel(NNSbwImagePtr image, int x, int y, int color); //set pixel color
int NNSbwGetPixel(NNSbwImagePtr image, int x, int y); //get pixel color

void NNSbwTriangle(NNSbwImagePtr image, int x1, int y1, int x2, int y2, int x3, int y3, int color); //draw triangle, based on Adafruit GFX library fillTriangle()
void NNSbwTriangleFilled(NNSbwImagePtr image, int x1, int y1, int x2, int y2, int x3, int y3, int color); //draw filled triangle, based on Adafruit GFX library fillTriangle()
void NNSbwPolygonFilled(NNSbwImagePtr image, NNSbwPointPtr pointArr, int pointCount, int color); //draw filled polygon
void NNSbwCircleFilled(NNSbwImagePtr image, int cx, int cy, int d, int color); //draw filled circle, based on Adafruit GFX library fillCircleHelper()
void NNSbwRectangleFilled(NNSbwImagePtr image, int x1, int y1, int x2, int y2, int color); //draw filled rectangle
void NNSbwLineHorizontal(NNSbwImagePtr image, int x, int y, int width, int color); //draw horizontal line
void NNSbwLineVertical(NNSbwImagePtr image, int x, int y, int height, int color); //draw vertical line
void NNSbwLine(NNSbwImagePtr image, int x1, int y1, int x2, int y2, int color); //draw line, based on Adafruit GFX library writeLine()
void NNSbwLineThick(NNSbwImagePtr image, float x1, float y1, float x2, float y2, float thickness, int color); //draw thick line
void NNSbwLineThickDepth(NNSbwImagePtr image, int bitmapWidth, int bitmapHeight, int startLayer, int endLayer, float x1, float y1, float x2, float y2, float thickness); //draw thick line that can cross multiple depth buffer image
void NNSbwLineThickVDepth(NNSbwImagePtr image, int bitmapWidth, int bitmapHeight, int startLayer, int endLayer, float x1, float y1, float x2, float y2, float toolDia, float toolAngle, float layerSpacing); //draw thick line with different start and end thickness that can cross multiple depth buffer image
void NNSbwArc(NNSbwImagePtr image, float cx, float cy, float width, float height, float start, float end, int color, int resolution); //draw arc
void NNSbwArcThick(NNSbwImagePtr image, float cx, float cy, float width, float height, float start, float end, float thickness, int color, int resolution); //draw thick arc
void NNSbwArcThickDepth(NNSbwImagePtr image, int bitmapWidth, int bitmapHeight, int startLayer, int endLayer, float cx, float cy, float width, float height, float start, float end, float thickness, int resolution); //draw thick arc that can cross multiple depth buffer image
void NNSbwArcThickVDepth(NNSbwImagePtr image, int bitmapWidth, int bitmapHeight, int startLayer, int endLayer, float cx, float cy, float width, float height, float startAngle, float endAngle, float toolDia, float toolAngle, float layerSpacing, int resolution); //draw thick arc with different start and end thickness that can cross multiple depth buffer image

//gdcode.h
extern int NNSclampInt(int num, int min, int max); //clamp int value
extern int NNSintFade(int low, int high, float fade); //proportional int gradian
extern int NNSgdImageColorFade(int* rgbArr1, int* rgbArr2, float fade); //proportional color gradian

//ncparser.h
extern double deg2rad(double deg); //convert degree to radian
extern double rad2deg(double rad); //convert radian to degree

#endif
