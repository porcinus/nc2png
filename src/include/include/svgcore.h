/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to SVG file format output.

Important:
- Very early implement, no deflate nor optimization here, just a bit of Javascript but that's it.
- Uncompressed Javascript found in ./include/svgcore_script.js

upd 0.4a ok
*/

#ifndef SVGCORE_H
#define SVGCORE_H

#include <stdio.h>
#include <new>
#include "ncparser.h"
#include <errno.h>


const char *svgJsScript = //placeholder javascript script for pan and zoom
"function viewboxGetParams(svgElement){ //get values array of viewbox attribute\n"
"    let currentViewbox = svgElement.getAttribute('viewBox');\n"
"    let params = currentViewbox.split(' ');\n"
"    for (let i = 0; i < 4; i++){params[i] = parseFloat(params[i]);}\n"
"    return params;\n"
"}\n"
"\n"
"function viewboxSetParams(svgElement, numArray){ //viewbox attribute from values array\n"
"    if (numArray[2] < 0.1){numArray[2] = 0.1;}\n"
"    if (numArray[3] < 0.1){numArray[3] = 0.1;}\n"
"    svgElement.setAttribute('viewBox', numArray.join(' '));\n"
"}"
"\n"
"function cssSetRootProperty(svgElement, propertyName, value){ //update root css property\n"
"    document.documentElement.style.setProperty(propertyName, value);\n"
"}"
"\n"
"function strokeScaling(svgBoundArray, viewBoxArray, strokeSize){ //compute new stroke size\n"
"    let scalingFactor = Math.min(svgBoundArray.width / viewBoxArray[2], svgBoundArray.height / viewBoxArray[3]);\n"
"    return strokeSize / scalingFactor * 5.;\n"
"}\n"
"\n"
"//svg\n"
"var svg = document.querySelector('svg');\n"
"\n"
"//backup initial viewport values\n"
"var viewBoxParamsInit = viewboxGetParams(svg);\n"
"var viewBoxMinSize = Math.min(viewBoxParamsInit[2], viewBoxParamsInit[3]);\n"
"\n"
"//mouse wheel scaling factor\n"
"var wheelScaler = 10.;\n"
"\n"
"//initial stroke width\n"
"var strokeDefaultWidthInit = .20;\n"
"var strokeGridWidthInit = .25;\n"
"var svgBound = svg.getBoundingClientRect();\n"
"cssSetRootProperty(svg, '--strokeWidthDefault', strokeScaling(svgBound, viewBoxParamsInit, strokeDefaultWidthInit));\n"
"cssSetRootProperty(svg, '--strokeWidthGrid', strokeScaling(svgBound, viewBoxParamsInit, strokeGridWidthInit));\n"
"\n"
"var panView = false;\n"
"svg.addEventListener('mousedown', function(evt){if (evt.button == 0){panView = true;}}, false); //mouse left button press, start view pan\n"
"svg.addEventListener('mouseup', function(evt){if (evt.button == 0){panView = false;}}, false); //mouse left button release, stop view pan\n"
"svg.addEventListener('mouseleave', function(){panView = false;}, false); //mouse left svg element, stop view pan\n"
"\n"
"svg.addEventListener('mousemove', function(evt){ //mouse move, pan view\n"
"    if (panView){\n"
"        let viewBoxParams = viewboxGetParams(svg);\n"
"        svgBound = svg.getBoundingClientRect();\n"
"        viewBoxParams[0] -= evt.movementX * (viewBoxParams[2] / svgBound.width);\n"
"        viewBoxParams[1] -= evt.movementY * (viewBoxParams[3] / svgBound.height);\n"
"        viewboxSetParams(svg, viewBoxParams); //update viewbox values\n"
"    }\n"
"}, false);\n"
"\n"
"svg.addEventListener('wheel', function(evt){ //mouse wheel event, zoom\n"
"    let viewBoxParams = viewboxGetParams(svg);\n"
"    let zoomDirection = (evt.deltaY > 0) ? 1 : -1;\n"
"    let wheelScalingW = (viewBoxParams[2] / wheelScaler);\n"
"    let wheelScalingH = (viewBoxParams[3] / wheelScaler);\n"
"\n"
"    viewBoxParams[2] += wheelScalingW * zoomDirection; //width\n"
"    viewBoxParams[3] += wheelScalingH * zoomDirection; //height\n"
"    viewboxSetParams(svg, viewBoxParams); //update viewbox values\n"
"\n"
"    //update stroke widths according to new viewbox 'zoom'\n"
"    svgBound = svg.getBoundingClientRect();\n"
"    cssSetRootProperty(svg, '--strokeWidthDefault', strokeScaling(svgBound, viewBoxParams, strokeDefaultWidthInit));\n"
"    cssSetRootProperty(svg, '--strokeWidthGrid', strokeScaling(svgBound, viewBoxParams, strokeGridWidthInit));\n"
"}, false);\n"
;


int* stdNewArrayResizeInt(int* array, size_t oldSize, size_t newSize); //resize a array created with 'new', WARNING: oldSize/newSize in bytes
char* rgb2htmlColor(char* str, unsigned int r, unsigned int g, unsigned int b); //convert rgb int color to html format
char* rgb2htmlColorFade(char* str, unsigned int* rgbArr1, unsigned int* rgbArr2, float fade); //proportional html color gradian

void svgDrawLine(FILE* handle, float x1, float y1, float x2, float y2, char* cssClass, float strokeWidth, char* strokeColor, bool strokeRounded);
void svgDrawLineDashed(FILE* handle, float x1, float y1, float x2, float y2, char* cssClass, float strokeWidth, float strokeDash, char* strokeColor);
void svgDrawCircle(FILE* handle, float cx, float cy, float r, char* cssClass, float strokeWidth, char* strokeColor);
void svgDrawCircleDashed(FILE* handle, float cx, float cy, float r, char* cssClass, float strokeWidth, float strokeDash, char* strokeColor);
void svgDrawRect(FILE* handle, float x, float y, float width, float height, char* cssClass, char* bgColor, float strokeWidth, char* strokeColor, bool strokeRounded);
void svgDrawRectDashed(FILE* handle, float x, float y, float width, float height, char* cssClass, char* bgColor, float strokeWidth, float strokeDash, char* strokeColor);
void svgDrawArrow(FILE* handle, float x, float y, float width, float height, float angle, bool rotateFromTip, char* cssClass, char* bgColor, float strokeWidth, char* strokeColor, bool strokeRounded);
void svgDrawArc(FILE* handle, float centerX, float centerY, float xSize, float ySize, float startAngle, float endAngle, float resolution, char* cssClass, char* bgColor, float strokeWidth, char* strokeColor, bool strokeRounded);
void svgDrawLineV(FILE* handle, float x1, float y1, float x2, float y2, char* cssClass, float strokeWidthStart, float strokeWidthEnd, char* strokeColor);
void svgDrawArcV(FILE* handle, float centerX, float centerY, float xSize, float ySize, float startAngle, float endAngle, float resolution, char* cssClass, char* bgColor, float strokeWidthStart, float strokeWidthEnd, char* strokeColor);

int svgPreview(char* file, int svgPrevArcRes, ncLineStruct* lines, ncToolStruct* tools, ncCommentStruct* comments, ncLimitStruct* limits, ncSummaryStruct* summary, ncArraySize* arrSizes, bool debugOutput); //generate image preview from ncparser data

//nc2png.h
extern unsigned int streamBufferSize; //nc2png.h: try to optimize file writing using bigger buffer

//ncparser.h
extern float numDiffFloat(float n1, float n2); //ncparser.h: absolute delta between 2 floats
extern double deg2rad(double deg); //convert degree to radian
extern bool inArrayInt(int* arr, int value, int arrSize); //check if int in array



#endif