/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to SVG file format output.

Important:
- Very early implement, no deflate nor optimization here, just a bit of Javascript but that's it.
- Uncompressed Javascript found in ./include/svgcore_script.js

upd 0.4a ok
*/

#include "svgcore.h"
#include "ncparser.h"


int* stdNewArrayResizeInt(int* array, size_t oldSize, size_t newSize){ //resize a array created with 'new', WARNING: oldSize/newSize in bytes
    if (array == nullptr){debug_stderr("Failed: array ptr is null\n"); return array;}
    int* newArray = new(std::nothrow)int[newSize];
    if (newArray == nullptr){debug_stderr("Failed to allocate new array\n"); return array;}
    memcpy(newArray, array, oldSize);
    delete []array;
    return newArray;
}

char* rgb2htmlColor(char* str, unsigned int r, unsigned int g, unsigned int b){ //convert rgb int color to html format
    sprintf(str, "#%02X%02X%02X", r, g, b);
    return str;
}

char* rgb2htmlColorFade(char* str, unsigned int* rgbArr1, unsigned int* rgbArr2, float fade){ //proportional html color gradian
    if (fade < 0.f || std::isnan(fade)){return rgb2htmlColor(str, rgbArr1[0], rgbArr1[1], rgbArr1[2]);}
    if (fade > 1.f){return rgb2htmlColor(str, rgbArr2[0], rgbArr2[1], rgbArr2[2]);}
    return rgb2htmlColor(str, rgbArr1[0] + (rgbArr2[0] - rgbArr1[0]) * fade, rgbArr1[1] + (rgbArr2[1] - rgbArr1[1]) * fade, rgbArr1[2] + (rgbArr2[2] - rgbArr1[2]) * fade);
}


void svgDrawLine(FILE* handle, float x1, float y1, float x2, float y2, char* cssClass, float strokeWidth, char* strokeColor, bool strokeRounded){
    if (handle == NULL){return;}
    fprintf(handle, "<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\"", x1, y1, x2, y2);
    if (cssClass != NULL && *cssClass != '\0'){fprintf(handle, " class=\"%s\"", cssClass);}
    if (numDiffFloat(0.f, strokeWidth) > 0.01){fprintf(handle, " stroke-width=\"%.2f\"", strokeWidth);}
    if (strokeColor != NULL && *strokeColor != '\0'){fprintf(handle, " stroke=\"%s\"", strokeColor);}
    if (strokeRounded){fprintf(handle, " stroke-linecap=\"round\"");}
    fprintf(handle, "/>\n");
}

void svgDrawLineDashed(FILE* handle, float x1, float y1, float x2, float y2, char* cssClass, float strokeWidth, float strokeDash, char* strokeColor){
    if (handle == NULL){return;}
    fprintf(handle, "<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" stroke-dasharray=\"%.2f\"", x1, y1, x2, y2, strokeDash);
    if (cssClass != NULL && *cssClass != '\0'){fprintf(handle, " class=\"%s\"", cssClass);}
    if (numDiffFloat(0.f, strokeWidth) > 0.01){fprintf(handle, " stroke-width=\"%.2f\"", strokeWidth);}
    if (strokeColor != NULL && *strokeColor != '\0'){fprintf(handle, " stroke=\"%s\"", strokeColor);}
    fprintf(handle, "/>\n");
}

void svgDrawCircle(FILE* handle, float cx, float cy, float r, char* cssClass, float strokeWidth, char* strokeColor){
    if (handle == NULL){return;}
    fprintf(handle, "<circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\"", cx, cy, r);
    if (cssClass != NULL && *cssClass != '\0'){fprintf(handle, " class=\"%s\"", cssClass);}
    if (numDiffFloat(0.f, strokeWidth) > 0.01){fprintf(handle, " stroke-width=\"%.2f\"", strokeWidth);}
    if (strokeColor != NULL && *strokeColor != '\0'){fprintf(handle, " stroke=\"%s\"", strokeColor);}
    fprintf(handle, "/>\n");
}

void svgDrawCircleDashed(FILE* handle, float cx, float cy, float r, char* cssClass, float strokeWidth, float strokeDash, char* strokeColor){
    if (handle == NULL){return;}
    fprintf(handle, "<circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" stroke-dasharray=\"%.2f\"", cx, cy, r, strokeDash);
    if (cssClass != NULL && *cssClass != '\0'){fprintf(handle, " class=\"%s\"", cssClass);}
    if (numDiffFloat(0.f, strokeWidth) > 0.01){fprintf(handle, " stroke-width=\"%.2f\"", strokeWidth);}
    if (strokeColor != NULL && *strokeColor != '\0'){fprintf(handle, " stroke=\"%s\"", strokeColor);}
    fprintf(handle, "/>\n");
}

void svgDrawRect(FILE* handle, float x, float y, float width, float height, char* cssClass, char* bgColor, float strokeWidth, char* strokeColor, bool strokeRounded){
    if (handle == NULL){return;}
    fprintf(handle, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" fill=\"%s\"", x, y, width, height, (bgColor == NULL) ? "none" : bgColor);
    if (cssClass != NULL && *cssClass != '\0'){fprintf(handle, " class=\"%s\"", cssClass);}
    if (numDiffFloat(0.f, strokeWidth) > 0.01){fprintf(handle, " stroke-width=\"%.2f\"", strokeWidth);}
    if (strokeColor != NULL && *strokeColor != '\0'){fprintf(handle, " stroke=\"%s\"", strokeColor);}
    if (strokeRounded){fprintf(handle, " stroke-linecap=\"round\"");}
    fprintf(handle, "/>\n");
}

void svgDrawRectDashed(FILE* handle, float x, float y, float width, float height, char* cssClass, char* bgColor, float strokeWidth, float strokeDash, char* strokeColor){
    if (handle == NULL){return;}
    fprintf(handle, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" stroke-dasharray=\"%.2f\" fill=\"%s\"", x, y, width, height, strokeDash, (bgColor == NULL) ? "none" : bgColor);
    if (cssClass != NULL && *cssClass != '\0'){fprintf(handle, " class=\"%s\"", cssClass);}
    if (numDiffFloat(0.f, strokeWidth) > 0.01){fprintf(handle, " stroke-width=\"%.2f\"", strokeWidth);}
    if (strokeColor != NULL && *strokeColor != '\0'){fprintf(handle, " stroke=\"%s\"", strokeColor);}
    fprintf(handle, "/>\n");
}

void svgDrawArrow(FILE* handle, float x, float y, float width, float height, float angle, bool rotateFromTip, char* cssClass, char* bgColor, float strokeWidth, char* strokeColor, bool strokeRounded){
    //rotateFromTip also set provided x/y to the arrow tip 
    if (handle == NULL){return;}
    float coord[12] = { //x,y
        rotateFromTip ? -width : 0, 0,
        rotateFromTip ? -width : 0, -height / 2,
        rotateFromTip ? 0 : width, 0,
        rotateFromTip ? -width : 0, 0,
        rotateFromTip ? -width : 0, height / 2,
        rotateFromTip ? 0 : width, 0
    };
    fprintf(handle, "<polygon fill=\"%s\" transform=\"translate(%.2f,%.2f)", (bgColor == NULL) ? "transparent" : bgColor, x, y);
    if (abs(0.F - angle) > 0.0001){fprintf(handle, " rotate(%.2f,0,0)", angle);}
    fprintf(handle, "\" points=\""); for (int i = 0; i < 12; i++){fprintf(handle, i ? ",%.2f" : "%.2f", coord[i]);} fprintf(handle, "\"");
    if (cssClass != NULL && *cssClass != '\0'){fprintf(handle, " class=\"%s\"", cssClass);}
    if (numDiffFloat(0.f, strokeWidth) > 0.01){fprintf(handle, " stroke-width=\"%.2f\"", strokeWidth);}
    if (strokeColor != NULL && *strokeColor != '\0'){fprintf(handle, " stroke=\"%s\"", strokeColor);}
    if (strokeRounded){fprintf(handle, " stroke-linecap=\"round\"");}
    fprintf(handle, "/>\n");
}

void svgDrawArc(FILE* handle, float centerX, float centerY, float xSize, float ySize, float startAngle, float endAngle, float resolution, char* cssClass, char* bgColor, float strokeWidth, char* strokeColor, bool strokeRounded){
    if (numDiffFloat(startAngle, endAngle) < 0.001 || xSize < 0.001 || ySize < 0.001){return;}
    if (resolution < 0.001){resolution = 0.001;}
    float a1 = deg2rad(startAngle), a2 = deg2rad(endAngle);
    float arcLenght = (abs(a2 - a1) * (xSize / 2) + abs(a2 - a1) * (ySize / 2)) / 2;
    float hWidth = xSize/2, hHeight = ySize/2;
    int steps = ceil(arcLenght / resolution) + 1;
    float stepangle = (a2 - a1) / (float)steps;
    for (int i = 0; i < steps + 1; i++){
        float x = centerX + cos(a1 + stepangle * i) * hWidth;
        float y = centerY + sin(a1 + stepangle * i) * hHeight;
        if (i == 0){fprintf(handle, "<path fill=\"%s\" d=\"M%.2f %.2f L", (bgColor == NULL) ? "none" : bgColor, x, y);} else {fprintf(handle, "%.2f %.2f ", x, y);}
    }
    fprintf(handle, "\"");
    if (cssClass != NULL && *cssClass != '\0'){fprintf(handle, " class=\"%s\"", cssClass);}
    if (numDiffFloat(0.f, strokeWidth) >= 0.01f){fprintf(handle, " stroke-width=\"%.2f\"", strokeWidth);}
    if (strokeColor != NULL && *strokeColor != '\0'){fprintf(handle, " stroke=\"%s\"", strokeColor);}
    if (strokeRounded){fprintf(handle, " stroke-linecap=\"round\"");}
    fprintf(handle, "/>\n");
}

void svgDrawLineV(FILE* handle, float x1, float y1, float x2, float y2, char* cssClass, float strokeWidthStart, float strokeWidthEnd, char* strokeColor){
    if (handle == NULL){return;}
    if (strokeWidthStart == 0.f){strokeWidthStart = 0.001f;}
    if (strokeWidthEnd == 0.f){strokeWidthEnd = 0.001f;}

    if (numDiffFloat(x1, x2) < 0.001 && numDiffFloat(y1, y2) < 0.001){ //start equal end point
        float radius = (strokeWidthStart > strokeWidthEnd) ? strokeWidthStart / 2 : strokeWidthEnd / 2;
        if (radius >= 0.001f){svgDrawCircle(handle, x1, y1, radius, cssClass, 0.f, strokeColor);}
        return;
    }
    if (numDiffFloat(strokeWidthStart, strokeWidthEnd) < 0.01){ //same stroke width
        svgDrawLine(handle, x1, y1, x2, y2, cssClass, strokeWidthStart, strokeColor, true);
        return;
    }
    if (cssClass == NULL && strokeColor == NULL){return;}

    bool validClass = false;
    bool validColor = false;
    if (cssClass != NULL && *cssClass != '\0'){validClass = true;}
    if (strokeColor != NULL && *strokeColor != '\0'){validColor = true;}
    if (!validClass && !validColor){return;}

    if(x2 - x1 <= 0){
        float tmpx = x1, tmpy = y1, tmpd = strokeWidthStart;
        x1 = x2;
        y1 = y2;
        x2 = tmpx;
        y2 = tmpy;
        strokeWidthStart = strokeWidthEnd;
        strokeWidthEnd = tmpd;
    }

    if (strokeWidthStart < 0.001){strokeWidthStart = 0.001;}
    if (strokeWidthEnd < 0.001){strokeWidthEnd = 0.001;}

    float hd1 = strokeWidthStart / 2, hd2 = strokeWidthEnd / 2, vx = x2 - x1, vy = y2 - y1;
    if (numDiffFloat(x1, x2) < 0.001){x2 += 0.001;}
    if (numDiffFloat(y1, y2) < 0.001){y2 += 0.001;}
    float centerdist = sqrt(vx * vx + vy * vy), relangle = acos((float)(hd1 - hd2) / centerdist), absangle = atan((float)vy / vx);
    float anglea = relangle - absangle, angleb = relangle + absangle;
    float sina = sin(anglea), sinb = sin(angleb), cosa = cos(anglea), cosb = cos(angleb);

    fprintf(handle, "<g stroke-width=\"0\" stroke=\"none\"");
    if (validClass){fprintf(handle, " class=\"%s\"", cssClass);}
    if (validColor){fprintf(handle, " fill=\"%s\"", strokeColor);}
    fprintf(handle, ">\n");

    fprintf(handle, "\t<circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\"/>\n", x1, y1, hd1);
    fprintf(handle, "\t<circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\"/>\n", x2, y2, hd2);
    fprintf(handle, "\t<path d=\"M%.2f %.2f L", x1 + hd1 * cosa, y1 - hd1 * sina);
    fprintf(handle, "%.2f %.2f ", x2 + hd2 * cosa, y2 - hd2 * sina);
    fprintf(handle, "%.2f %.2f ", x2 + hd2 * cosb, y2 + hd2 * sinb);
    fprintf(handle, "%.2f %.2f\"/>\n", x1 + hd1 * cosb, y1 + hd1 * sinb);

    fprintf(handle, "</g>\n");
}

void svgDrawArcV(FILE* handle, float centerX, float centerY, float xSize, float ySize, float startAngle, float endAngle, float resolution, char* cssClass, char* bgColor, float strokeWidthStart, float strokeWidthEnd, char* strokeColor){
    if (numDiffFloat(startAngle, endAngle) < 0.001f || xSize < 0.001f || ySize < 0.001f){return;}
    if (strokeWidthStart == 0.f){strokeWidthStart = 0.001f;}
    if (strokeWidthEnd == 0.f){strokeWidthEnd = 0.001f;}
    if (resolution < 0.001f){resolution = 0.001f;}

    if (numDiffFloat(strokeWidthStart, strokeWidthEnd) < 0.01){ //same stroke width
        if (strokeWidthStart < 0.01){return;}
        svgDrawArc(handle, centerX, centerY, xSize, ySize, startAngle, endAngle, resolution, cssClass, bgColor, strokeWidthStart, strokeColor, true);
        return;
    }

    if (cssClass == NULL && strokeColor == NULL){return;}

    bool validClass = false;
    bool validColor = false;
    if (cssClass != NULL && *cssClass != '\0'){validClass = true;}
    if (strokeColor != NULL && *strokeColor != '\0'){validColor = true;}
    if (!validClass && !validColor){return;}

    float a1 = deg2rad(startAngle), a2 = deg2rad(endAngle);
    float arcLenght = (abs(a2 - a1) * (xSize / 2) + abs(a2 - a1) * (ySize / 2)) / 2;
    float hWidth = xSize / 2, hHeight = ySize / 2;
    int steps = ceil(arcLenght / resolution) + 1;
    float stepangle = (a2 - a1) / (float)steps, stepd = (strokeWidthEnd - strokeWidthStart) / (float)steps;

    fprintf(handle, "<g stroke-width=\"0\" stroke=\"none\"");
    if (validClass){fprintf(handle, " class=\"%s\"", cssClass);}
    if (validColor){fprintf(handle, " fill=\"%s\"", strokeColor);}
    fprintf(handle, ">\n");

    fprintf(handle, "\t<circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\"/>\n", centerX + cos(a1) * hWidth, centerY + sin(a1) * hHeight, strokeWidthStart / 2.);
    fprintf(handle, "\t<circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\"/>\n", centerX + cos(a1 + stepangle * steps) * hWidth, centerY + sin(a1 + stepangle * steps) * hHeight, strokeWidthEnd / 2.);

    //compute poligon points
    struct tmpPosStruct {float x = 0, y = 0;};
    int pointsArrSize = (steps + 1) * 2 + 1;
    tmpPosStruct *pointsArr = new(std::nothrow)tmpPosStruct[pointsArrSize];

    if (pointsArr != nullptr){
        debug_stderr("pointsArr allocated (size:%d)\n", pointsArrSize);
    } else {
        debug_stderr("failed to allocate pointsArr (size:%d)\n", pointsArrSize);
        return;
    }

    for (int i = 0, j = pointsArrSize - 1; i < steps + 1; i++, j--){
        float tmpa = a1 + (stepangle * i);
        float tmpx =  cos(tmpa);
        float tmpy = sin(tmpa);
        tmpa = (strokeWidthStart + stepd * i) / 2;
        pointsArr[i].x = centerX + tmpx * (hWidth - tmpa);
        pointsArr[i].y = centerY + tmpy * (hHeight - tmpa);
        pointsArr[j].x = centerX + tmpx * (hWidth + tmpa);
        pointsArr[j].y = centerY + tmpy * (hHeight + tmpa);
    }

    for (int i = 0; i < pointsArrSize; i++){
        if (i == 0){fprintf(handle, "\t<path d=\"M%.2f %.2f L", pointsArr[i].x, pointsArr[i].y);} else {fprintf(handle, "%.2f %.2f ", pointsArr[i].x, pointsArr[i].y);}
    }

    delete []pointsArr;

    fprintf(handle, "\"");
    if (cssClass != NULL && *cssClass != '\0'){fprintf(handle, " class=\"%s\"", cssClass);}
    if (strokeColor != NULL && *strokeColor != '\0'){fprintf(handle, " fill=\"%s\"", strokeColor);}
    fprintf(handle, "/>\n");

    fprintf(handle, "</g>\n");
}


int svgPreview(char* file, int svgPrevArcRes, ncLineStruct* lines, ncToolStruct* tools, ncCommentStruct* comments, ncLimitStruct* limits, ncSummaryStruct* summary, ncArraySize* arrSizes, bool debugOutput){ //generate image preview from ncparser data
    bool debugBack = debug;
    debug = debugOutput;

    char previewFilePath[PATH_MAX]; sprintf(previewFilePath, "%s.svg.html", file);
    FILE *svgFileHandle = fopen(previewFilePath, "wb");
    if (svgFileHandle == NULL){return -ENOENT;} //ERR1 : failed to create file

    //increase write buffer
    char *svgFileBuffer = new(std::nothrow)char[streamBufferSize + 1];
    if (svgFileBuffer != nullptr){
        debug_stderr("svgFileBuffer allocated\n");
        setvbuf(svgFileHandle, svgFileBuffer, _IOFBF, streamBufferSize);
    } else {debug_stderr("Warning: failed to allocate svgFileBuffer\n");}

    float svgPrevMargin = 10.f;

    //workspace limits
    float svgPrevXmin = ((limits->xMin > 0.) ? 0. : limits->xMin) - svgPrevMargin;
    float svgPrevXmax = ((limits->xMax < 0.) ? 0. : limits->xMax) + svgPrevMargin;
    float svgPrevYmin = ((limits->yMin > 0.) ? 0. : limits->yMin) - svgPrevMargin;
    float svgPrevYmax = ((limits->yMax < 0.) ? 0. : limits->yMax) + svgPrevMargin;
    float svgPrevWidth = abs(svgPrevXmax - svgPrevXmin);
    float svgPrevHeight = abs(svgPrevYmax - svgPrevYmin);

    //normalized scale relative 100% max scale
    float svgPrevScale = ((svgPrevWidth > svgPrevHeight) ? svgPrevHeight : svgPrevWidth) / 100.f;

    char svgColorBackgroundStr[8] = {0}; rgb2htmlColor(svgColorBackgroundStr, 0, 0, 0); //background color
    char svgColorAxisStr[8] = {0}; rgb2htmlColor(svgColorAxisStr, 180, 180, 180); //axis color
    char svgColorGridStr[8] = {0}; rgb2htmlColor(svgColorGridStr, 70, 70, 70); //grid color
    char svgColorSubGridStr[8] = {0}; rgb2htmlColor(svgColorSubGridStr, 35, 35, 35); //subgrid color
    char svgColorWorkStr[8] = {0}; rgb2htmlColor(svgColorWorkStr, 0, 0, 255); //work toolpath color
    char svgColorRadiusStr[8] = {0}; rgb2htmlColor(svgColorRadiusStr, 0, 255, 0); //circular toolpath color
    char svgColorFastStr[8] = {0}; rgb2htmlColor(svgColorFastStr, 255, 0, 0); //fast toolpath color
    unsigned int svgColorToolLowerArr[] = {0, 80, 80}; //lower position tool color
    unsigned int svgColorToolHigherArr[] = {0, 200, 200}; //higher position tool color
    char svgColorToolStr[8] = {0}; rgb2htmlColor(svgColorToolStr, 0, 200, 200); //tool string color
    char svgColorLimitsStr[8] = {0}; rgb2htmlColor(svgColorLimitsStr, 255, 255, 0); //limits color
    char svgColorLimitsLinesStr[8] = {0}; rgb2htmlColor(svgColorLimitsLinesStr, 180, 180, 0); //limits lines color

    float defaultStrokeWidth = 0.2f;
    float gridStrokeWidth = 0.25f;

    //header
    fprintf(svgFileHandle, 
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<body>\n"
    "<svg viewBox=\"%.2f %.2f %.2f %.2f\" preserveAspectRatio=\"xMinYMin meet\" style=\"background-color:black\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n",
    svgPrevXmin, -svgPrevYmax, svgPrevWidth, svgPrevHeight);

    //css style
    fprintf(svgFileHandle, 
    "<style type=\"text/css\">\n"
    "\t:root {\n"
    "\t\t--strokeWidthDefault: %.2f;\n"
    "\t\t--strokeWidthGrid: %.2f;\n"
    "\t}\n"
    , defaultStrokeWidth, gridStrokeWidth); 

    fprintf(svgFileHandle, "\t.subgrid {stroke: %s; stroke-width: var(--strokeWidthDefault);}\n", svgColorSubGridStr);
    fprintf(svgFileHandle, "\t.grid {stroke: %s; stroke-width: var(--strokeWidthGrid);}\n", svgColorGridStr);
    fprintf(svgFileHandle, "\t.limits {stroke: %s; stroke-width: var(--strokeWidthDefault);}\n", svgColorLimitsStr);
    fprintf(svgFileHandle, "\t.axis {stroke: %s; stroke-width: var(--strokeWidthDefault);}\n", svgColorAxisStr);

    fprintf(svgFileHandle, "\t.fast {stroke: %s; stroke-width: var(--strokeWidthDefault);}\n", svgColorFastStr);
    fprintf(svgFileHandle, "\t.work {stroke: %s; stroke-width: var(--strokeWidthDefault);}\n", svgColorWorkStr);
    fprintf(svgFileHandle, "\t.circ {stroke: %s; stroke-width: var(--strokeWidthDefault);}\n", svgColorRadiusStr);
    fprintf(svgFileHandle, "\t.drill {stroke: %s; stroke-width: var(--strokeWidthDefault);}\n", svgColorWorkStr);

    //generate classes for stroke depth color
    bool toolStyling = false;
    if (summary->tools > 0){
        unsigned int tmpDepthArrSize = 100, tmpIndex = 0; //array size and index position
        int *tmpDepthArr = new(std::nothrow)int[tmpDepthArrSize]; //start with arbitrary size
        if (tmpDepthArr != nullptr){
            debug_stderr("tmpDepthArr[%u] allocated\n", tmpDepthArrSize);

            char svgColorTool[8];
            rgb2htmlColorFade(svgColorTool, svgColorToolLowerArr, svgColorToolHigherArr, (0.f - limits->zMinWork) / (limits->zMaxWork - limits->zMinWork));
            fprintf(svgFileHandle, "\t.depth0 {stroke: %s;}\n", svgColorTool); //avoid glitch at Z0

            for (unsigned int line = 1; line < arrSizes->lineStrucLimit; line++){
                if (lines[line].g == 0 || tools[lines[line].tool].angle > 0.01){continue;} //only include work lines without non engraving tool
                int tmpNum = (int)(lines[line].z * 100.f);
                if (!inArrayInt(tmpDepthArr, tmpNum, tmpDepthArrSize)){
                    *(tmpDepthArr + tmpIndex++) = tmpNum;
                    rgb2htmlColorFade(svgColorTool, svgColorToolLowerArr, svgColorToolHigherArr, (lines[line].z - limits->zMinWork) / (limits->zMaxWork - limits->zMinWork));
                    fprintf(svgFileHandle, "\t.depth%d {stroke: %s;}\n", tmpNum, svgColorTool);
                    if (tmpIndex == tmpDepthArrSize){ //need to resize the array
                        size_t oldSize = tmpDepthArrSize * sizeof(unsigned int);
                        size_t newSize = (tmpDepthArrSize + 100) * sizeof(unsigned int);
                        tmpDepthArr = stdNewArrayResizeInt(tmpDepthArr, oldSize, newSize);
                        if (tmpDepthArr != nullptr){
                            tmpDepthArrSize += 100;
                            debug_stderr("tmpDepthArr resized to %u\n", tmpDepthArrSize);
                        } else {
                            debug_stderr("failed to resize tmpDepthArr to %u\n", tmpDepthArrSize);
                            break;
                        }
                    }
                }
            }
            if (tmpDepthArr != nullptr){
                delete []tmpDepthArr;
                toolStyling = true;
                debug_stderr("tmpDepthArr index was %u\n", tmpIndex);
            }
        } else {debug_stderr("Warning: failed to allocate tmpDepthArr, tool styling disabled\n");}

    //generate classes for tool<>stroke size
        for (unsigned int i = 0; i < arrSizes->toolStrucLimit; i++){
            if (tools[i].num != -1 && numDiffFloat(0.f, tools[i].diameter) > 0.001){
                fprintf(svgFileHandle, "\t.dia%d {stroke-width: %.2f;}\n", (int)(tools[i].diameter * 100.f), tools[i].diameter);
            }
        }
    }

    fprintf(svgFileHandle, "</style>\n");

    //patterns
    //note: using rect because some browser doesn't properly implement overflow tag
    fprintf(svgFileHandle, "<defs>\n");

    fprintf(svgFileHandle, //subgrid
    "\t<pattern id=\"subgrid\" patternUnits=\"userSpaceOnUse\" width=\"1\" height=\"1\">\n"
    "\t\t<rect x=\"0\" y=\"0\" width=\"1\" height=\"1\" fill=\"none\" class=\"subgrid\"/>\n"
    "\t</pattern>\n");

    fprintf(svgFileHandle, //grid
    "\t<pattern id=\"grid\" patternUnits=\"userSpaceOnUse\" width=\"10\" height=\"10\">\n"
    "\t\t<rect x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"none\" class=\"grid\"/>\n"
    "\t</pattern>\n");

    fprintf(svgFileHandle, "</defs>\n");

    //background, subgrid, grid
    fprintf(svgFileHandle, "<!--subgrid, grid-->\n");
    {
        //upper-lower rounding to ease grid/subgrid drawing
        float tmpXmin = floorf(svgPrevXmin / 10.f) * 10.f;
        float tmpXmax = ceilf(svgPrevXmax / 10.f) * 10.f;
        float tmpYmin = floorf(svgPrevYmin / 10.f) * 10.f;
        float tmpYmax = ceilf(svgPrevYmax / 10.f) * 10.f;
        float tmpWidth = abs(tmpXmax - tmpXmin);
        float tmpHeight = abs(tmpYmax - tmpYmin);

        char tmpStrSubgrid[] = "url(#subgrid)";
        char tmpStrGrid[] = "url(#grid)";
        
        svgDrawRect(svgFileHandle, tmpXmin, -tmpYmax, tmpWidth, tmpHeight, NULL, tmpStrSubgrid, 0, NULL, false); //subgrid
        svgDrawRect(svgFileHandle, tmpXmin, -tmpYmax, tmpWidth, tmpHeight, NULL, tmpStrGrid, 0, NULL, false); //grid
    }

    //limits
    fprintf(svgFileHandle, "<!--limits-->\n");
    float strokeDashWidth = svgPrevScale * 1.f; //1% dash spacing
    float limitsWidth = abs(limits->xMax - limits->xMin), limitsHeight = abs(limits->yMax - limits->yMin);
    svgDrawRectDashed(svgFileHandle, limits->xMin, -(limits->yMax), limitsWidth, limitsHeight, (char*)"limits", NULL, 0.f, strokeDashWidth, NULL);

    //draw toolpaths with width
    unsigned int commentLast = UINT_MAX;
    bool firstComment = true;
    if(summary->tools > 0){
        fprintf(svgFileHandle, "<!--cut path-->\n");
        fprintf(svgFileHandle, "<g stroke-linecap=\"round\" shape-rendering=\"crispEdges\">\n");

        bool firstDepth = true, groupUsed = false;
        float toolDia = 1, dz = .0, lastdZ = .0, lastDepth = -9999999999.;
        char classStr[16] = "", strokeColor[8] = "";

        for (unsigned int line = 1; line < arrSizes->lineStrucLimit; line++){
            int g = lines[line].g;
            if (g > 0 && lines[line].tool != -1){
                toolDia = tools[lines[line].tool].diameter;

                if (tools[lines[line].tool].angle <= 0.01){
                    if ((lines[line].comment != commentLast || firstComment)){
                        commentLast = lines[line].comment;
                        if (!firstComment){fprintf(svgFileHandle, "\t</g>\n");} else {firstComment = false;}
                        fprintf(svgFileHandle, "\t<g id=\"cut%u\" class=\"dia%d\">\n", commentLast, (int)(toolDia * 100.f));
                    }

                    if (lines[line].z != lastDepth || firstDepth){
                        if (groupUsed){
                            if (!firstDepth){fprintf(svgFileHandle, "\t\t</g>\n");}
                            groupUsed = false;
                        }
                        if (firstDepth){firstDepth = false;}
                        lastDepth = lines[line].z;

                        if (toolStyling){
                            sprintf(classStr, "depth%d", (int)(lastDepth * 100.f));
                        } else {
                            rgb2htmlColorFade(strokeColor, svgColorToolLowerArr, svgColorToolHigherArr, (lastDepth - limits->zMinWork) / (limits->zMaxWork - limits->zMinWork));
                        }

                        if (line + 1 < arrSizes->lineStrucLimit){
                            if (lines[line + 1].z == lastDepth){ //more than one entity at current depth
                                fprintf(svgFileHandle, "\t\t<g");
                                if (toolStyling){
                                    fprintf(svgFileHandle, " class=\"%s\"", classStr);
                                    classStr[0] = '\0';
                                } else {
                                    fprintf(svgFileHandle, " stroke=\"%s\"", strokeColor);
                                    strokeColor[0] = '\0';
                                }
                                fprintf(svgFileHandle, ">\n");
                                groupUsed = true;
                            }
                        }
                    }

                    fprintf(svgFileHandle, "\t\t\t");

                    if (g == 2 || g == 3){
                        if (numDiffFloat(lines[line].startAngle, lines[line].endAngle) > 0.01f){
                            svgDrawArc(svgFileHandle, lines[line].i, -(lines[line].j), lines[line].radius * 2, lines[line].radius * 2, lines[line].startAngle, lines[line].endAngle, svgPrevArcRes, classStr, NULL, 0.f, strokeColor, false);
                        }
                        if (numDiffFloat(lines[line].startAngle1, lines[line].endAngle1) > 0.01f){
                            svgDrawArc(svgFileHandle, lines[line].i, -(lines[line].j), lines[line].radius * 2, lines[line].radius * 2, lines[line].startAngle1, lines[line].endAngle1, svgPrevArcRes, classStr, NULL, 0.f, strokeColor, false);
                        }
                    } else if (g == 81 || g == 82 || g == 83){
                        svgDrawCircle(svgFileHandle, lines[line].x, -(lines[line].y), 0.01f, classStr, 0.f, strokeColor);
                    } else if (g == 1){
                        svgDrawLine(svgFileHandle, lines[line-1].x, -(lines[line-1].y), lines[line].x, -(lines[line].y), classStr, 0.f, strokeColor, false);
                    }
                } else {
                    if (lines[line].z != lastDepth || firstDepth){
                        lastDepth = lines[line].z;
                        rgb2htmlColorFade(strokeColor, svgColorToolLowerArr, svgColorToolHigherArr, (lastDepth - limits->zMinWork) / (limits->zMaxWork - limits->zMinWork));
                        if (firstDepth){firstDepth = false;}
                    }

                    dz = (tan(deg2rad(tools[lines[line].tool].angle)) * abs (lines[line].z)) * 2;
                    if (dz < 0.001f){
                        dz = 0.001f;
                    } else if (dz > toolDia * 2){
                        dz = toolDia * 2;
                    }

                    if (g == 2){
                        if (numDiffFloat(lines[line].startAngle, lines[line].endAngle) > 0.01f){
                            svgDrawArcV(svgFileHandle, lines[line].i, -(lines[line].j), lines[line].radius * 2, lines[line].radius * 2, lines[line].startAngle, lines[line].endAngle, svgPrevArcRes, classStr, NULL, lastdZ, dz, strokeColor);
                        }
                        if (numDiffFloat(lines[line].startAngle1, lines[line].endAngle1) > 0.01f){
                            svgDrawArcV(svgFileHandle, lines[line].i, -(lines[line].j), lines[line].radius * 2, lines[line].radius * 2, lines[line].startAngle1, lines[line].endAngle1, svgPrevArcRes, classStr, NULL, lastdZ, dz, strokeColor);
                        }
                    } else  if (g == 3){
                        if (numDiffFloat(lines[line].startAngle, lines[line].endAngle) > 0.01f){
                            svgDrawArcV(svgFileHandle, lines[line].i, -(lines[line].j), lines[line].radius * 2, lines[line].radius * 2, lines[line].startAngle, lines[line].endAngle, svgPrevArcRes, classStr, NULL, dz, lastdZ, strokeColor);
                        }
                        if (numDiffFloat(lines[line].startAngle1, lines[line].endAngle1) > 0.01f){
                            svgDrawArcV(svgFileHandle, lines[line].i, -(lines[line].j), lines[line].radius * 2, lines[line].radius * 2, lines[line].startAngle1, lines[line].endAngle1, svgPrevArcRes, classStr, NULL, dz, lastdZ, strokeColor);
                        }
                    } else if (g == 81 || g == 82 || g == 83){
                        fprintf(svgFileHandle, "<circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\"/>\n", lines[line].x, -(lines[line].y), dz / 2);
                    } else if (g == 1){
                        svgDrawLineV(svgFileHandle, lines[line-1].x, -(lines[line-1].y), lines[line].x, -(lines[line].y), NULL, lastdZ, dz, strokeColor);
                    }
                    lastdZ = dz;
                }
            }
        }
        
        if (groupUsed){fprintf(svgFileHandle, "\t\t</g>\n");}
        if (!firstComment){fprintf(svgFileHandle, "\t</g>\n");}
        fprintf(svgFileHandle, "</g>\n");
    }

    //axis, origin
    {
        float originSignRadius = svgPrevScale * 1.25f; //origin sign radius of 1.25% of document
        float originArrowWidth = originSignRadius * 2 * 0.75f; //axis arrow width
        float originArrowHeight = originSignRadius * 2 * 0.6f; //axis arrow height

        fprintf(svgFileHandle,
        "<!--axis, origin-->\n"
        "<g class=\"axis\">\n");

        svgDrawLine(svgFileHandle, svgPrevXmin, 0, svgPrevXmax, 0, NULL, 0, NULL, false); //x axis
        svgDrawLine(svgFileHandle, 0, -svgPrevYmin, 0, -svgPrevYmax, NULL, 0, NULL, false); //y axis

        fprintf(svgFileHandle, "<circle cx=\"0\" cy=\"0\" r=\"%.2f\" fill=\"%s\"/>\n", originSignRadius, svgColorBackgroundStr);
        fprintf(svgFileHandle, "<path d=\"M 0 %.2f A %.2f %.2f 90 0 1 -%.2f 0 L 0 0\" fill=\"%s\"/>\n", originSignRadius, originSignRadius, originSignRadius, originSignRadius, svgColorAxisStr); //bottom left
        fprintf(svgFileHandle, "<path d=\"M 0 -%.2f A %.2f %.2f 270 0 1 %.2f 0 L 0 0\" fill=\"%s\"/>\n", originSignRadius, originSignRadius, originSignRadius, originSignRadius, svgColorAxisStr); //top right

        svgDrawArrow(svgFileHandle, svgPrevXmax, 0, originArrowWidth, originArrowHeight, 0, true, NULL, svgColorAxisStr, 0, NULL, true); //x arrows
        svgDrawArrow(svgFileHandle, 0, -svgPrevYmax, originArrowWidth, originArrowHeight, 270, true, NULL, svgColorAxisStr, 0, NULL, true); //y arrows

        fprintf(svgFileHandle, "</g>\n");
    }

    //work toolpath lines
    commentLast = UINT_MAX;
    firstComment = true;
    int gLast = -1;
    fprintf(svgFileHandle, "<!--work path-->\n");
    fprintf(svgFileHandle, "<g>\n");

    for (unsigned int line = 1; line < arrSizes->lineStrucLimit; line++){
        int g = lines[line].g;
        if (g > 0){
            if ((lines[line].comment != commentLast || g != gLast || firstComment)){
                commentLast = lines[line].comment;
                if (!firstComment){fprintf(svgFileHandle, "\t</g>\n");} else {firstComment = false;}

                char workTypeStr[6];
                if (g == 2 || g == 3){
                    strcpy(workTypeStr, "circ");
                } else if (g == 81 || g == 82 || g == 83){
                    strcpy(workTypeStr, "drill");
                } else {
                    strcpy(workTypeStr, "work");
                }
                fprintf(svgFileHandle, "\t<g id=\"%s%u\" class=\"%s\">\n", workTypeStr, commentLast, workTypeStr);
            }

            fprintf(svgFileHandle, "\t\t");

            if (g == 2 || g == 3){
                if (numDiffFloat(lines[line].startAngle, lines[line].endAngle) > 0.01f){
                    svgDrawArc(svgFileHandle, lines[line].i, -(lines[line].j), lines[line].radius * 2, lines[line].radius * 2, lines[line].startAngle, lines[line].endAngle, svgPrevArcRes, NULL, NULL, 0.f, NULL, false);
                }
                if (numDiffFloat(lines[line].startAngle1, lines[line].endAngle1) > 0.01f){
                    svgDrawArc(svgFileHandle, lines[line].i, -(lines[line].j), lines[line].radius * 2, lines[line].radius * 2, lines[line].startAngle1, lines[line].endAngle1, svgPrevArcRes, NULL, NULL, 0.f, NULL, false);
                }
            } else if (g == 81 || g == 82 || g == 83){
                svgDrawCircle(svgFileHandle, lines[line].x, -(lines[line].y), 0.01f, NULL, 0.f, NULL);
            } else if (g == 1){
                svgDrawLine(svgFileHandle, lines[line-1].x, -(lines[line-1].y), lines[line].x, -(lines[line].y), NULL, 0.f, NULL, false);
            }

            gLast = g;
        }
    }

    if (!firstComment){fprintf(svgFileHandle, "\t</g>\n");}
    fprintf(svgFileHandle, "</g>\n");

    //fast toolpath lines
    commentLast = UINT_MAX;
    firstComment = true;
    fprintf(svgFileHandle, "<!--fast path-->\n");
    fprintf(svgFileHandle, "<g class=\"fast\">\n");
    for (unsigned int line = 1; line < arrSizes->lineStrucLimit; line++){
        if (lines[line].g == 0){
            if ((lines[line].comment != commentLast || firstComment)){
                commentLast = lines[line].comment;
                if (!firstComment){fprintf(svgFileHandle, "\t</g>\n");} else {firstComment = false;}
                fprintf(svgFileHandle, "\t<g id=\"fast%u\">\n", commentLast);
            }
            fprintf(svgFileHandle, "\t\t");
            svgDrawLine(svgFileHandle, lines[line-1].x, -(lines[line-1].y), lines[line].x, -(lines[line].y), NULL, 0.f, NULL, false);
        }
    }
    if (!firstComment){fprintf(svgFileHandle, "\t</g>\n");}
    fprintf(svgFileHandle, "</g>\n");

    //info

    //ui script

    //footer
    //fprintf(svgFileHandle,
    //"</svg>\n"
    //"<script type=\"text/javascript\" src=\"svgcore_script.js\"></script>\n"
    //"</body>\n"
    //"</html>\n");

    fprintf(svgFileHandle,
    "</svg>\n"
    "<script type=\"text/javascript\">%s</script>\n"
    "</body>\n"
    "</html>\n", svgJsScript);

    fclose(svgFileHandle);
    if (svgFileBuffer != nullptr){delete []svgFileBuffer;}
    debug = debugBack;
    return 0;
}

