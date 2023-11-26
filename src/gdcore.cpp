/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to libGD, generate preview from a custom structs (ncparser.h/cpp).

upd 0.4a ok
*/

#include "gdcore_language.h"
#include "gdcore.h"

//#define depthBufferImageCount 256 //depthmap layers count

int NNSclampInt(int num, int min, int max){ //clamp int value
    if (num < min){return min;}
    if (num > max){return max;}
    return num;
}

float NNSclampFloat(float num, float min, float max){ //clamp float value
    if (num < min){return min;}
    if (num > max){return max;}
    return num;
}

int NNSintFade(int low, int high, float fade){ //proportional int gradian, fade:0.0 to 1.0
    if (fade <= 0.f){return low;} else if (fade >= 1.f){return high;}
    return low + (high - low) * fade;
}

float NNSfloatFade(float low, float high, float fade){ //proportional float gradian, fade:0.0 to 1.0
    if (fade <= 0.f){return low;} else if (fade >= 1.f){return high;}
    return low + (high - low) * fade;
}

int NNSintFadeNoClamp(int low, int high, float fade){ //proportional int gradian, fade:0.0 to 1.0, no clamping
    return low + (high - low) * fade;
}

float NNSfloatFadeNoClamp(float low, float high, float fade){ //proportional float gradian, fade:0.0 to 1.0, no clamping
    return low + (high - low) * fade;
}

void NNSintArrFade(int* arrDest, int* arr1, int* arr2, int size, float fade){ //proportional int array gradian, fade:0.0 to 1.0
    if (fade <= 0.f){
        for (int i = 0; i < size; i++){*(arrDest + i) = *(arr1 + i);}
    } else if (fade >= 1.f){
        for (int i = 0; i < size; i++){*(arrDest + i) = *(arr2 + i);}
    } else {
        for (int i = 0; i < size; i++){*(arrDest + i) = *(arr1 + i) + (*(arr2 + i) - *(arr1 + i)) * fade;}
    }
}

int NNSgdImageColorFade(int* rgbArr1, int* rgbArr2, float fade){ //proportional color gradian, fade:0.0 to 1.0
    if (fade <= 0.f){return gdTrueColorAlpha(rgbArr1[0], rgbArr1[1], rgbArr1[2], gdAlphaOpaque);}
    if (fade >= 1.f){return gdTrueColorAlpha(rgbArr2[0], rgbArr2[1], rgbArr2[2], gdAlphaOpaque);}
    //todo implement rgb fading instead of raw int fading
    return gdTrueColorAlpha(
        (int)(rgbArr1[0] + (rgbArr2[0] - rgbArr1[0]) * fade),
        (int)(rgbArr1[1] + (rgbArr2[1] - rgbArr1[1]) * fade),
        (int)(rgbArr1[2] + (rgbArr2[2] - rgbArr1[2]) * fade),
        gdAlphaOpaque);
}

void NNSgdImageDepth2Color(gdImagePtr image, int* rgbArrLow, int* rgbArrHigh){ //convert gdImage from 0-255 colors (highest-lowest) to given arrays (r,g,b), alpha will be 0 or 127
    if (image->trueColor != 1){return;}
    
    //colors trust table
    int* colorTable = new(std::nothrow)int[256];
    if (colorTable == nullptr){return;}
    for (int i = 0; i < 256; i++){
        colorTable[i] = gdTrueColorAlpha(NNSintFade(rgbArrLow[0], rgbArrHigh[0], i / 255.f), NNSintFade(rgbArrLow[1], rgbArrHigh[1], i / 255.f), NNSintFade(rgbArrLow[2], rgbArrHigh[2], i / 255.f), gdAlphaOpaque);
    }
    
    int w = image->sx, h = image->sy;
    for (int y = 0; y < h; y++){ //partially based on gd.c gdImageCopy() function
        for (int x = 0; x < w; x++){
            int colorStart = image->tpixels[y][x];
            if (gdTrueColorGetAlpha(colorStart) != gdAlphaTransparent){image->tpixels[y][x] = colorTable[gdTrueColorGetRed(colorStart)];}
        }
    }

    delete []colorTable;
}

void NNSgdImageFill(gdImagePtr image, int color){ //gdImageFill() replacement to avoid transparent color glitch
    int w = image->sx, h = image->sy;
    if (image->trueColor){
        for (int y = 0; y < h; y++){
            for (int x = 0; x < w; x++){image->tpixels[y][x] = color;}
        }
    } else {
        for (int y = 0; y < h; y++){
            for (int x = 0; x < w; x++){image->pixels[y][x] = color;}
        }
    }
}

void NNSgdImageTriangleFilled(gdImagePtr image, int x1, int y1, int x2, int y2, int x3, int y3, int color){ //draw filled triangle, based on Adafruit GFX library fillTriangle()
    //sort coordinates by Y order (y3 >= y2 >= y1)
    int tmpInt;
    if (y1 > y2){tmpInt = y1; y1 = y2; y2 = tmpInt; tmpInt = x1; x1 = x2; x2 = tmpInt;}
    if (y2 > y3){tmpInt = y3; y3 = y2; y2 = tmpInt; tmpInt = x3; x3 = x2; x2 = tmpInt;}
    if (y1 > y2){tmpInt = y1; y1 = y2; y2 = tmpInt; tmpInt = x1; x1 = x2; x2 = tmpInt;}
    
    int a, b;
    if (y1 == y3){ //handle awkward all-on-same-line case as its own thing
        a = b = x1;
        if (x2 < a){a = x2;} else if (x2 > b){b = x2;}
        if (x3 < a){a = x3;} else if (x3 > b){b = x3;}
        for (; a <= b; a++){gdImageSetPixel(image, a, y1, color);}
        return;
    }
    
    int dx11 = x2 - x1, dy11 = y2 - y1, dx12 = x3 - x1, dy12 = y3 - y1, dx22 = x3 - x2, dy22 = y3 - y2;
    int sa = 0, sb = 0, last;
    //for upper part of triangle, find scanline crossings for segments 0-1 and 0-2.
    //if y2=y3 (flat-bottomed triangle), the scanline y2 is included here (and second loop will be skipped, avoiding a /0error there), otherwise scanline y2 is skipped here and handledin the second loop...
    //which also avoids a /0 error here if y1=y2 (flat-topped triangle)
    if (y2 == y3){last = y2;} else {last = y2 - 1;} //include y2 scanline or skip it
    
    int y;
    for (y = y1; y <= last; y++){
        a = x1 + sa / dy11; b = x1 + sb / dy12;
        sa += dx11; sb += dx12;
        if (a > b){tmpInt = a; a = b; b = tmpInt;}
        for (; a <= b; a++){gdImageSetPixel(image, a, y, color);}
    }
    
    //for lower part of triangle, find scanline crossings for segments
    //0-2 and 1-2.  This loop is skipped if y2=y3.
    sa = dx22 * (y - y2);
    sb = dx12 * (y - y1);
    for (; y <= y3; y++) {
        a = x2 + sa / dy22; b = x1 + sb / dy12;
        sa += dx22; sb += dx12;
        if (a > b){tmpInt = a; a = b; b = tmpInt;}
        for (; a <= b; a++){gdImageSetPixel(image, a, y, color);}
    }
}

void NNSgdImagePolygonFilled(gdImagePtr image, gdPointPtr pointArr, int pointCount, int color){ //draw filled polygon, bypass LibGD complex function
    if (pointCount <= 0){return;}
    if (pointCount == 1){gdImageSetPixel(image, pointArr[0].x, pointArr[0].y, color); return;}
    if (pointCount == 2){gdImageLine(image, pointArr[0].x, pointArr[0].y, pointArr[1].x, pointArr[1].y, color); return;}
    if (pointCount == 3){NNSgdImageTriangleFilled(image, pointArr[0].x, pointArr[0].y, pointArr[1].x, pointArr[1].y, pointArr[2].x, pointArr[2].y, color); return;}
    
    bool upTriangle = true;
    int upLast = 0, downLast = pointCount - 1;
    while ((downLast - upLast) > 0){
        if (upTriangle){
            NNSgdImageTriangleFilled(image, pointArr[upLast].x, pointArr[upLast].y, pointArr[upLast + 1].x, pointArr[upLast + 1].y, pointArr[downLast].x, pointArr[downLast].y, color);
            upLast++;
        } else {
            NNSgdImageTriangleFilled(image, pointArr[downLast].x, pointArr[downLast].y, pointArr[downLast - 1].x, pointArr[downLast - 1].y, pointArr[upLast].x, pointArr[upLast].y, color);
            downLast--;
        }
        upTriangle = !upTriangle;
    }
}

void NNSgdImageLineDashed(gdImagePtr image, int x1, int y1, int x2, int y2, int color1, int color2, int space){ //draw dashed line the proper way, GD one is glitched
    int style[space * 2 + 1];
    for (int i = 0; i < space; i++){style[i] = color1;}
    for (int i = 0; i < space; i++){style[space + i] = color2;}
    gdImageSetStyle(image, style, space * 2);
    gdImageLine(image, x1, y1, x2, y2, gdStyled);
}

void NNSgdImageArc(gdImagePtr image, float cx, float cy, float width, float height, float start, float end, int color, int resolution){ //draw arc, allow precision definition, GD one draw one line per degree
    if (resolution < 1){resolution = 20;}
    float a1 = deg2rad(start), a2 = deg2rad(end), arclenght = abs(a2 - a1) * (height / 2), hwidth = width/2, hheight = height/2;
    int step = (float)ceil(arclenght / resolution) + 1;
    float stepangle = (a2 - a1) / step;
    float lastx = cx + cos(a1) * hwidth, lasty = cy + sin(a1) * hheight, x, y;
    for (int i = 0; i < step + 1; i++){
        x = cx + cos(a1 + stepangle * i) * hwidth;
        y = cy + sin(a1 + stepangle * i) * hheight;
        gdImageLine(image, x, y, lastx, lasty, color);
        lastx = x; lasty = y;
    }
}

void NNSgdImageArrow(gdImagePtr image, float x, float y, float lenght, float width, float height, int dir, int color, bool filled){ //draw simple arrow
    gdPoint pointsArr[3];
    if (dir == 0){ //x+
        gdImageLine(image, x, y, x + lenght, y, color);
        pointsArr[0].x = x + lenght - width;
        pointsArr[0].y = y - height / 2;
        pointsArr[1].x = x + lenght;
        pointsArr[1].y = y;
        pointsArr[2].x = x + lenght - width;
        pointsArr[2].y = y + height / 2;
    } else if (dir == 1){ //x-
        gdImageLine(image, x, y, x - lenght, y, color);
        pointsArr[0].x = x - lenght + width;
        pointsArr[0].y = y - height / 2;
        pointsArr[1].x = x - lenght;
        pointsArr[1].y = y;
        pointsArr[2].x = x - lenght + width;
        pointsArr[2].y = y + height / 2;
    } else if (dir == 2){ //y+
        gdImageLine(image, x, y, x, y + lenght, color);
        pointsArr[0].x = x - height / 2;
        pointsArr[0].y = y + lenght - width;
        pointsArr[1].x = x;
        pointsArr[1].y = y + lenght;
        pointsArr[2].x = x + height / 2;
        pointsArr[2].y = y + lenght - width;
    } else { //y-
        gdImageLine(image, x, y, x, y - lenght, color);
        pointsArr[0].x = x - height / 2;
        pointsArr[0].y = y - lenght + width;
        pointsArr[1].x = x;
        pointsArr[1].y = y - lenght;
        pointsArr[2].x = x + height / 2;
        pointsArr[2].y = y - lenght + width;
    }
    if (filled){
        NNSgdImageTriangleFilled(image, pointsArr[0].x, pointsArr[0].y, pointsArr[1].x, pointsArr[1].y, pointsArr[2].x, pointsArr[2].y, color);
    } else {
        gdImageLine(image, pointsArr[0].x, pointsArr[0].y, pointsArr[1].x, pointsArr[1].y, color);
        gdImageLine(image, pointsArr[1].x, pointsArr[1].y, pointsArr[2].x, pointsArr[2].y, color);
        gdImageLine(image, pointsArr[2].x, pointsArr[2].y, pointsArr[0].x, pointsArr[0].y, color);
    }
}


void sec2charArr(char* arr, double time){ //convert seconds to char array in format : XXsec / XXmin / XXh XXmin
    int hours = 0, minutes = 0;
    int seconds = floor(time);
    if (seconds < 60){sprintf(arr, "%dsec", seconds);
    } else if (seconds < 3600){
        minutes = floor(seconds / 60);
        seconds = ceil(seconds % 60); if (seconds > 59){seconds = 0; minutes++;}
        if (seconds == 0){sprintf(arr, "%dmin", minutes);
        } else {sprintf(arr, "%dmin %dsec", minutes, seconds);}
    } else {
        hours = floor(seconds / 3600);
        minutes = ceil((seconds % 3600) / 60); if (minutes > 59){minutes = 0; hours++;}
        if (minutes == 0){sprintf(arr, "%dh", hours);
        } else {sprintf(arr, "%dh %dmin", hours, minutes);}
    }
}

int NNSPNGaddPPM(char* file, unsigned int ppm){ //write specific PPM to PNG file, can be done via libpng but it is faster that way
    FILE* tmpHandle = fopen(file, "rb+"); //file stream
    if (tmpHandle != NULL){ //valid file stream
        char tmpBuffer[5]; //signature read buffer
        if (fgets(tmpBuffer , 5 , tmpHandle) != NULL){
            if (strcmp(tmpBuffer, "\x89PNG\0") != 0){ //security, not png signature
                fclose(tmpHandle);
                return -ENOENT;
            }
        } else { //failed to open file
            fclose(tmpHandle);
            return -ENOENT;
        }

        char pHYsIdent[] = "pHYs"; char currChar; unsigned int identPos = 0, seek = 5;
        fseek (tmpHandle, seek , SEEK_SET); //seek after signature
        do {
            seek++; currChar = (char) fgetc (tmpHandle);
            if ((char)currChar == (char)pHYsIdent[identPos]){identPos++;} else {identPos = 0;}
            if (currChar == EOF || seek > 255){fclose(tmpHandle); return -ENOENT;} //failed to found pHYs block after 255 bytes
        } while (identPos < 4);
        fseek (tmpHandle, seek-4 , SEEK_SET); //seek before pHYs block start
        
        char tmpBufferPPM[14] = {'p', 'H', 'Y', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\1', '\0'}; //pHYs block w/o crc32
        
        char *tmpPtr = (char*) &ppm; //ppm pointer to swap endian
        tmpBufferPPM[4] = tmpPtr[3]; tmpBufferPPM[5] = tmpPtr[2]; tmpBufferPPM[6] = tmpPtr[1]; tmpBufferPPM[7] = tmpPtr[0]; //swap endian x ppm
        tmpBufferPPM[8] = tmpPtr[3]; tmpBufferPPM[9] = tmpPtr[2]; tmpBufferPPM[10] = tmpPtr[1]; tmpBufferPPM[11] = tmpPtr[0]; //swap endian y ppm
        fwrite(&tmpBufferPPM, 1, 13, tmpHandle); //write ppm block
        
        unsigned long crc = crc32(0, (Bytef*) &tmpBufferPPM, 13); //use crc32 funct from zlib
        tmpPtr = (char*) &crc;
        tmpBufferPPM[0] = tmpPtr[3]; //swap endian
        tmpBufferPPM[1] = tmpPtr[2];
        tmpBufferPPM[2] = tmpPtr[1];
        tmpBufferPPM[3] = tmpPtr[0];
        fwrite(&tmpBufferPPM, 1, 4, tmpHandle); //write crc
        
        fclose(tmpHandle); //close stream
        return 0; //success
    }
    return -ENOENT; //failed
}


int gdPreview(char* file, int gdPrevWidth, int gdPrevArcRes, bool exportDepthMap, ncLineStruct* lines, ncToolStruct* tools, ncCommentStruct* comments, ncLimitStruct* limits, ncSummaryStruct* summary, ncArraySize* arrSizes, bool debugOutput){ //generate image preview from ncparser data
    bool debugBack = debug;
    debug = debugOutput;
    
    FILE *gdFileHandle; //file handle
    char previewFilePath[PATH_MAX];
    double benchmarkStart = 0.;
    
    int gdPrevHeight = gdPrevWidth;
    int gdPrevAxisFontWidth = gdFontGiant->w, gdPrevAxisFontHeight = gdFontGiant->h; //px
    int gdPrevStrFontWidth = gdFontSmall->w, gdPrevStrFontHeight = gdFontSmall->h; //px
    
    int gdPrevXmin = 0, gdPrevXmax = 0, gdPrevYmin = 0, gdPrevYmax = 0, gdPrevMargin = 10, gdPrevGrid = 10, gdPrevSubGrid = 1, tmpInt = 0; //mm
    float gdPrevScale = 1.f;
    
    if (0.f > limits->xMin){gdPrevXmin = limits->xMin;}
    if (0.f < limits->xMax){gdPrevXmax = limits->xMax;}
    if (0.f > limits->yMin){gdPrevYmin = limits->yMin;}
    if (0.f < limits->yMax){gdPrevYmax = limits->yMax;}
    
    if (abs(gdPrevXmax - gdPrevXmin) > abs(gdPrevYmax - gdPrevYmin)){ //compute height, width > height
        gdPrevHeight = gdPrevWidth / (float)((abs(gdPrevXmax - gdPrevXmin) + gdPrevMargin * 2) / (float)(abs(gdPrevYmax - gdPrevYmin) + gdPrevMargin * 2));
        gdPrevScale = (float)gdPrevWidth / (float)(abs(gdPrevXmax - gdPrevXmin) + gdPrevMargin * 2);
    } else { //height > width
        gdPrevHeight = gdPrevWidth;
        gdPrevScale = (float)gdPrevWidth / (float)(abs(gdPrevYmax - gdPrevYmin) + gdPrevMargin * 2);
    }
    
	float gdPrevXoffset = abs(gdPrevXmin) + gdPrevMargin, gdPrevYoffset = abs(gdPrevYmin) + gdPrevMargin;
    int gdPrevWidthFull = gdPrevWidth + (gdPrevStrFontWidth + 10) * 2;
    int gdPrevTxtLines = 6;
    if (summary->tools > 0){gdPrevTxtLines++;}
    
    gdImagePtr gdPrevImage = gdImageCreateTrueColor(gdPrevWidthFull, gdPrevHeight + gdPrevStrFontHeight * gdPrevTxtLines + 4);
    if (gdPrevImage == NULL){return -ENOMEM;} //fail to allocate gdImagePtr
    
    if (summary->tools > 0){gdPrevTxtLines--;}
    char strBuffer[1024];
    
    //gd colors
    int gdPrevColorBackground = gdTrueColorAlpha(0, 0, 0, gdAlphaOpaque); //background color
    int gdPrevColorAxis = gdTrueColorAlpha(180, 180, 180, gdAlphaOpaque); //axis color
    int gdPrevColorGrid = gdTrueColorAlpha(70, 70, 70, gdAlphaOpaque); //grid color
    int gdPrevColorSubGrid = gdTrueColorAlpha(35, 35, 35, gdAlphaOpaque); //subgrid color
    int gdPrevColorWork = gdTrueColorAlpha(0, 0, 255, gdAlphaOpaque); //work toolpath color
    int gdPrevColorRadius = gdTrueColorAlpha(0, 255, 0, gdAlphaOpaque); //circular toolpath color
    int gdPrevColorFast = gdTrueColorAlpha(255, 0, 0, gdAlphaOpaque); //fast toolpath color
    int gdPrevColorToolLowerArr[] = {0, 80, 80}; //lower position tool color
    int gdPrevColorToolHigherArr[] = {0, 200, 200}; //higher position tool color
    int gdPrevColorTool = gdTrueColorAlpha(gdPrevColorToolHigherArr[0], gdPrevColorToolHigherArr[1], gdPrevColorToolHigherArr[2], gdAlphaOpaque); //generic tool color
    int gdPrevColorToolStr = gdTrueColorAlpha(gdPrevColorToolHigherArr[0], gdPrevColorToolHigherArr[1], gdPrevColorToolHigherArr[2], gdAlphaOpaque); //tool string color
    int gdPrevColorLimits = gdTrueColorAlpha(255, 255, 0, gdAlphaOpaque); //limits color
    int gdPrevColorLimitsLines = gdTrueColorAlpha(180, 180, 0, gdAlphaOpaque); //limits lines color
    
    //reset image background
    NNSgdImageFill(gdPrevImage, gdPrevColorBackground);
    
    //origins
    int gdPrevOrigX = gdPrevXoffset * gdPrevScale;
    int gdPrevOrigY = gdPrevHeight - gdPrevYoffset * gdPrevScale;
    
    //draw subgrid
    float gdPrevSubGridScaled = (float)gdPrevSubGrid * gdPrevScale;
    if (gdPrevSubGridScaled > 5.){
        for (float gdPrevX1 = gdPrevOrigX; gdPrevX1 < gdPrevWidth; gdPrevX1 += gdPrevSubGridScaled){ //y+ grid
            gdImageLine(gdPrevImage, gdPrevX1, 0, gdPrevX1, gdPrevHeight, gdPrevColorSubGrid);
        }
        for (float gdPrevX1 = gdPrevOrigX; gdPrevX1 > 0; gdPrevX1 -= gdPrevSubGridScaled){ //y- grid
            gdImageLine(gdPrevImage, gdPrevX1, 0, gdPrevX1, gdPrevHeight, gdPrevColorSubGrid);
        }
        for (float gdPrevY1 = gdPrevOrigY; gdPrevY1 < gdPrevHeight; gdPrevY1 += gdPrevSubGridScaled){ //x+ grid
            gdImageLine(gdPrevImage, 0, gdPrevY1, gdPrevWidth, gdPrevY1, gdPrevColorSubGrid);
        }
        for (float gdPrevY1 = gdPrevOrigY; gdPrevY1 > 0; gdPrevY1 -= gdPrevSubGridScaled){ //x- grid
            gdImageLine(gdPrevImage, 0, gdPrevY1, gdPrevWidth, gdPrevY1, gdPrevColorSubGrid);
        }
    }
    
    //draw grid
    float gdPrevGrid_scaled = (float)gdPrevGrid * gdPrevScale;
    for (float gdPrevX1 = gdPrevOrigX; gdPrevX1 < gdPrevWidth; gdPrevX1 += gdPrevGrid_scaled){ //y+ grid
        gdImageLine(gdPrevImage, gdPrevX1, 0, gdPrevX1, gdPrevHeight, gdPrevColorGrid);
    }
    for (float gdPrevX1 = gdPrevOrigX; gdPrevX1 > 0; gdPrevX1 -= gdPrevGrid_scaled){ //y- grid
        gdImageLine(gdPrevImage, gdPrevX1, 0, gdPrevX1, gdPrevHeight, gdPrevColorGrid);
    }
    for (float gdPrevY1 = gdPrevOrigY; gdPrevY1 < gdPrevHeight; gdPrevY1 += gdPrevGrid_scaled){ //x+ grid
        gdImageLine(gdPrevImage, 0, gdPrevY1, gdPrevWidth, gdPrevY1, gdPrevColorGrid);
    }
    for (float gdPrevY1 = gdPrevOrigY; gdPrevY1 > 0; gdPrevY1 -= gdPrevGrid_scaled){ //x- grid
        gdImageLine(gdPrevImage, 0, gdPrevY1, gdPrevWidth, gdPrevY1, gdPrevColorGrid);
    }
    
    //limits boundary
    int gdPrevXminPx = (float)(limits->xMin + gdPrevXoffset) * gdPrevScale;
    int gdPrevXmaxPx = (float)(limits->xMax + gdPrevXoffset) * gdPrevScale;
    int gdPrevYminPx = gdPrevHeight - (float)(limits->yMin + gdPrevYoffset) * gdPrevScale;
    int gdPrevYmaxPx = gdPrevHeight - (float)(limits->yMax + gdPrevYoffset) * gdPrevScale;
    NNSgdImageLineDashed(gdPrevImage, gdPrevXminPx, gdPrevYmaxPx, gdPrevXminPx, gdPrevHeight, gdPrevColorLimitsLines, gdTransparent, 4);
    NNSgdImageLineDashed(gdPrevImage, gdPrevXmaxPx, gdPrevYmaxPx, gdPrevXmaxPx, gdPrevHeight, gdPrevColorLimitsLines, gdTransparent, 4);
    NNSgdImageLineDashed(gdPrevImage, gdPrevXminPx, gdPrevYminPx, gdPrevWidth, gdPrevYminPx, gdPrevColorLimitsLines, gdTransparent, 4);
    NNSgdImageLineDashed(gdPrevImage, gdPrevXminPx, gdPrevYmaxPx, gdPrevWidth, gdPrevYmaxPx, gdPrevColorLimitsLines, gdTransparent, 4);
    
    //draw toolpaths with width
    if(summary->tools > 0){
        NNSbwImagePtr bwDepthImage = NNSbwImageCreate(gdPrevWidth, gdPrevHeight);
        if (bwDepthImage != nullptr){
            //progress bar
            printfTerm(strGDCore[STR_GDCORE::GDCORE_PROGRESS][language], 0, arrSizes->lineStrucLimit);
            int progressPosition = 0;

            benchmarkStart = get_time_double();

            //float depthFade; 
            int depthIndexStart, depthIndexEnd;
            float toolDia = 1.f, toolAngle = 0.f, zWorkDiff = limits->zMaxWork - limits->zMinWork, layerSpacing = abs(zWorkDiff / 256) * gdPrevScale;

            for (unsigned int line = 1; line < arrSizes->lineStrucLimit; line++, progressPosition++){
                int g = lines[line].g;
                if (g != -1 && lines[line].tool != -1){
                    depthIndexStart = 255 - (255 * ((lines[line - 1].z - limits->zMinWork) / zWorkDiff));
                    depthIndexEnd = 255 - (255 * ((lines[line].z - limits->zMinWork) / zWorkDiff));

                    toolDia = tools[lines[line].tool].diameter * gdPrevScale;
                    toolAngle = tools[lines[line].tool].angle;

                    if (toolAngle < 0.01f){
                        if (g < 2){
                            float x1 = (lines[line - 1].x + gdPrevXoffset) * gdPrevScale, y1 = gdPrevHeight - (lines[line - 1].y + gdPrevYoffset) * gdPrevScale;
                            float x2 = (lines[line].x + gdPrevXoffset) * gdPrevScale, y2 = gdPrevHeight - (lines[line].y + gdPrevYoffset) * gdPrevScale;
                            NNSbwLineThickDepth(bwDepthImage, gdPrevWidth, gdPrevHeight, depthIndexStart, depthIndexEnd, x1, y1, x2, y2, toolDia);
                            goto drawLoopEnd;
                        }

                        if (g == 2 || g == 3){
                            float i = (lines[line].i + gdPrevXoffset) * gdPrevScale, j = gdPrevHeight - (lines[line].j + gdPrevYoffset) * gdPrevScale;
                            float radius = lines[line].radius * gdPrevScale * 2;
                            if (numDiffFloat(lines[line].startAngle, lines[line].endAngle) >= 1.f){
                                NNSbwArcThickDepth(bwDepthImage, gdPrevWidth, gdPrevHeight, depthIndexStart, depthIndexEnd, i, j, radius, radius, (g == 2) ? lines[line].startAngle : lines[line].endAngle, (g == 2) ? lines[line].endAngle : lines[line].startAngle, toolDia, gdPrevArcRes);
                            }
                            if (numDiffFloat(lines[line].startAngle1, lines[line].endAngle1) >= 1.f){
                                NNSbwArcThickDepth(bwDepthImage, gdPrevWidth, gdPrevHeight, depthIndexStart, depthIndexEnd, i, j, radius, radius, (g == 2) ? lines[line].startAngle1 : lines[line].endAngle1, (g == 2) ? lines[line].endAngle1 : lines[line].startAngle1, toolDia, gdPrevArcRes);
                            }
                            goto drawLoopEnd;
                        }
                        
                        if (g == 81 || g == 82 || g == 83){
                            float x2 = (lines[line].x + gdPrevXoffset) * gdPrevScale, y2 = gdPrevHeight - (lines[line].y + gdPrevYoffset) * gdPrevScale;
                            NNSbwCircleFilled(bwDepthImage, x2, y2, toolDia, depthIndexEnd);
                            goto drawLoopEnd;
                        }

                        goto drawLoopEnd;
                    }
                    
                    //V tool
                    if (g < 2){
                        float x1 = (lines[line - 1].x + gdPrevXoffset) * gdPrevScale, y1 = gdPrevHeight - (lines[line - 1].y + gdPrevYoffset) * gdPrevScale;
                        float x2 = (lines[line].x + gdPrevXoffset) * gdPrevScale, y2 = gdPrevHeight - (lines[line].y + gdPrevYoffset) * gdPrevScale;
                        NNSbwLineThickVDepth(bwDepthImage, gdPrevWidth, gdPrevHeight, depthIndexStart, depthIndexEnd, x1, y1, x2, y2, toolDia, toolAngle, layerSpacing);
                        goto drawLoopEnd;
                    }

                    if (g == 2 || g == 3){
                        float i = (lines[line].i + gdPrevXoffset) * gdPrevScale, j = gdPrevHeight - (lines[line].j + gdPrevYoffset) * gdPrevScale;
                        float radius = lines[line].radius * gdPrevScale * 2;
                        if (numDiffFloat(lines[line].startAngle, lines[line].endAngle) >= 1.f){
                            NNSbwArcThickVDepth(bwDepthImage, gdPrevWidth, gdPrevHeight, depthIndexStart, depthIndexEnd, i, j, radius, radius, (g == 2) ? lines[line].startAngle : lines[line].endAngle, (g == 2) ? lines[line].endAngle : lines[line].startAngle, toolDia, toolAngle, layerSpacing, gdPrevArcRes);
                        }
                        if (numDiffFloat(lines[line].startAngle1, lines[line].endAngle1) >= 1.f){
                            NNSbwArcThickVDepth(bwDepthImage, gdPrevWidth, gdPrevHeight, depthIndexStart, depthIndexEnd, i, j, radius, radius, (g == 2) ? lines[line].startAngle1 : lines[line].endAngle1, (g == 2) ? lines[line].endAngle1 : lines[line].startAngle1, toolDia, toolAngle, layerSpacing, gdPrevArcRes);
                        }
                        goto drawLoopEnd;
                    }
                    
                    if (g == 81 || g == 82 || g == 83){
                        float x2 = (lines[line].x + gdPrevXoffset) * gdPrevScale, y2 = gdPrevHeight - (lines[line].y + gdPrevYoffset) * gdPrevScale;
                        NNSbwLineThickVDepth(bwDepthImage, gdPrevWidth, gdPrevHeight, depthIndexStart, depthIndexEnd, x2, y2, x2, y2, toolDia, toolAngle, layerSpacing);
                        goto drawLoopEnd;
                    }
                }

                drawLoopEnd:;
                if (progressPosition > 999){
                    printf("\033[0G");
                    printfTerm(strGDCore[STR_GDCORE::GDCORE_PROGRESS][language], line, arrSizes->lineStrucLimit);
                    progressPosition = 0;
                }
            }
            
            printf("\033[0G\033[2K"); //remove progress bar
        }

        debug_stderr("toolpath cut duration : %.04lfs\n", get_time_double() - benchmarkStart);
        debug_stderr("bwPixelDrawn:%ld\n", bwPixelDrawn);
        
        //build depth image
        if (bwDepthImage != nullptr){
            benchmarkStart = get_time_double();
            gdImagePtr gdPrevDepthCombinedImage = gdImageCreateTrueColor(gdPrevWidth, gdPrevHeight);
            if (gdPrevDepthCombinedImage != NULL){
                gdImageSaveAlpha(gdPrevDepthCombinedImage, true);
                NNSgdImageFill(gdPrevDepthCombinedImage, gdTrueColorAlpha(255, 255, 255, gdAlphaTransparent));
                
                NNSbwImage2gdGreyscaleCopy(bwDepthImage, gdPrevDepthCombinedImage);
                NNSbwImageDestroy(bwDepthImage); //cleanup useless layers

                if (exportDepthMap){
                    sprintf(previewFilePath, "%s.depth.png", file);
                    gdFileHandle = fopen(previewFilePath, "wb");
                    if (gdFileHandle != NULL){
                        gdImagePng(gdPrevDepthCombinedImage, gdFileHandle);
                        fclose(gdFileHandle);
                        NNSPNGaddPPM(previewFilePath, (unsigned int)(gdPrevScale * 1000.)); //write proper ppm
                    }
                }
                
                //convert gray scale depth map to color
                NNSgdImageDepth2Color(gdPrevDepthCombinedImage, gdPrevColorToolLowerArr, gdPrevColorToolHigherArr);
                
                //copy colored depth image to main preview
                gdImageCopy(gdPrevImage, gdPrevDepthCombinedImage, 0, 0, 0, 0, gdPrevWidth, gdPrevHeight);
                
                //export colored depth map
                if (debugOutput){
                    sprintf(previewFilePath, "%s.depthColor.png", file);
                    gdFileHandle = fopen(previewFilePath, "wb");
                    if (gdFileHandle != NULL){
                        gdImagePng(gdPrevDepthCombinedImage, gdFileHandle);
                        fclose(gdFileHandle);
                        NNSPNGaddPPM(previewFilePath, (unsigned int)(gdPrevScale * 1000.)); //write proper ppm
                    }
                }
            }
            
            if (gdPrevDepthCombinedImage != NULL){gdImageDestroy(gdPrevDepthCombinedImage);}
            debug_stderr("depth map duration : %.04lfs\n", get_time_double() - benchmarkStart);
        }

        //tools string
        sprintf(strBuffer, "%s(s) : ", strGDCore[STR_GDCORE::GDCORE_TOOL][language]);
        for (unsigned int i = 0; i < arrSizes->toolStrucLimit; i++){
            if (tools[i].diameter > 0.){
                char strBuffer1[64];
                if (tools[i].angle > 0.){
                    sprintf(strBuffer1, " [\x01 %.2fmm V%.1f\xB0]", tools[i].diameter, tools[i].angle);
                } else {
                    sprintf(strBuffer1, " [\x01 %.2fmm]", tools[i].diameter);
                }
                strcat(strBuffer, strBuffer1);
            }
        }
        gdImageLine(gdPrevImage, 0, gdPrevHeight + gdPrevStrFontHeight * gdPrevTxtLines + 3, gdPrevWidthFull, gdPrevHeight + gdPrevStrFontHeight * gdPrevTxtLines + 3, gdPrevColorSubGrid);
        int gdPrevToolStrWidth = strlen(strBuffer) * gdPrevStrFontWidth;
        int gdPrevToolStrX = (gdPrevWidthFull - gdPrevToolStrWidth) / 2;
        gdImageString(gdPrevImage, gdFontSmall, gdPrevToolStrX, gdPrevHeight + gdPrevStrFontHeight * gdPrevTxtLines + 3, (unsigned char*)strBuffer, gdPrevColorToolStr);
    }
    
    //draw x axis
    gdImageLine(gdPrevImage, 0, gdPrevOrigY, gdPrevWidth-gdPrevAxisFontWidth-2, gdPrevOrigY, gdPrevColorAxis);
    NNSgdImageArrow(gdPrevImage, gdPrevWidth-gdPrevAxisFontWidth-2, gdPrevOrigY, 0, 10, 8, 0, gdPrevColorAxis, true);
    gdImageFilledRectangle(gdPrevImage, gdPrevWidth-gdPrevAxisFontWidth-1, gdPrevOrigY-gdPrevAxisFontHeight/2, gdPrevWidth, gdPrevOrigY+gdPrevAxisFontHeight/2, gdPrevColorBackground); //background cleanup
    gdImageString(gdPrevImage, gdFontGiant, gdPrevWidth-gdPrevAxisFontWidth, gdPrevOrigY-gdPrevAxisFontHeight/2, (unsigned char*)"X", gdPrevColorAxis); //text
    
    //draw y axis
    gdImageLine(gdPrevImage, gdPrevOrigX, gdPrevAxisFontHeight+2, gdPrevOrigX, gdPrevHeight, gdPrevColorAxis);
    NNSgdImageArrow(gdPrevImage, gdPrevOrigX, gdPrevAxisFontHeight+2, 0, 10, 8, 3, gdPrevColorAxis, true);
    gdImageFilledRectangle(gdPrevImage, gdPrevOrigX-gdPrevAxisFontWidth/2-1, 0, gdPrevOrigX+gdPrevAxisFontWidth/2+1, gdPrevAxisFontHeight, gdPrevColorBackground); //background cleanup
    gdImageString(gdPrevImage, gdFontGiant, gdPrevOrigX-gdPrevAxisFontWidth/2+1, 0, (unsigned char*)"Y", gdPrevColorAxis); //text
    
    //draw origin
    gdImageEllipse(gdPrevImage, gdPrevOrigX, gdPrevOrigY, 15, 15, gdPrevColorAxis);
    gdImageFilledArc(gdPrevImage, gdPrevOrigX, gdPrevOrigY, 15, 15, -90, 0, gdPrevColorAxis, gdPie);
    gdImageFilledArc(gdPrevImage, gdPrevOrigX, gdPrevOrigY, 15, 15, 90, 180, gdPrevColorAxis, gdPie);
    
    //watermark
    sprintf(strBuffer, "NC2PNG v%s", programversion);
    tmpInt = (strlen(strBuffer) * gdPrevStrFontWidth) / 2;
    gdImageFilledRectangle(gdPrevImage, gdPrevWidth/2-tmpInt-2, gdPrevHeight-gdPrevStrFontHeight, gdPrevWidth/2+tmpInt+1, gdPrevHeight, gdPrevColorBackground); //background cleanup
    gdImageString(gdPrevImage, gdFontSmall, gdPrevWidth/2-tmpInt, gdPrevHeight-gdPrevStrFontHeight, (unsigned char*)strBuffer, gdPrevColorGrid);
    
    //work toolpath lines
    benchmarkStart = get_time_double();
    for (unsigned int line = 1; line < arrSizes->lineStrucLimit; line++){
        int g = lines[line].g;
        if (g > 0){
            if (g == 2 || g == 3){
                float i = (lines[line].i + gdPrevXoffset) * gdPrevScale, j = gdPrevHeight - (lines[line].j + gdPrevYoffset) * gdPrevScale;
                float radius = lines[line].radius * gdPrevScale * 2;
                gdImageSetAntiAliased(gdPrevImage, gdPrevColorRadius);
                if (numDiffFloat(lines[line].startAngle, lines[line].endAngle) > 1.){
                    NNSgdImageArc(gdPrevImage, i, j, radius, radius, lines[line].startAngle, lines[line].endAngle, gdAntiAliased, gdPrevArcRes);
                }
                if (numDiffFloat(lines[line].startAngle1, lines[line].endAngle1) > 1.){
                    NNSgdImageArc(gdPrevImage, i, j, radius, radius, lines[line].startAngle1, lines[line].endAngle1, gdAntiAliased, gdPrevArcRes);
                }
                continue;
            }

            float x1 = (lines[line - 1].x + gdPrevXoffset) * gdPrevScale, y1 = gdPrevHeight - (lines[line - 1].y + gdPrevYoffset) * gdPrevScale;
            float x2 = (lines[line].x + gdPrevXoffset) * gdPrevScale, y2 = gdPrevHeight - (lines[line].y + gdPrevYoffset) * gdPrevScale;
            if (g == 81 || g == 82 || g == 83){
                gdImageSetAntiAliased(gdPrevImage, gdPrevColorFast);
                gdImageLine(gdPrevImage, x1, y1, x2, y2, gdAntiAliased);
                continue;
            }

            if (g == 1){
                gdImageSetAntiAliased(gdPrevImage, gdPrevColorWork);
                gdImageLine(gdPrevImage, x1, y1, x2, y2, gdAntiAliased);
            }
        }
    }
    debug_stderr("work path duration : %.04lfs\n", get_time_double() - benchmarkStart);
    
    //fast toolpath lines
    benchmarkStart = get_time_double();
    gdImageSetAntiAliased(gdPrevImage, gdPrevColorFast);
    for (unsigned int line = 1; line < arrSizes->lineStrucLimit; line++){
        if (lines[line].g == 0){
            float x1 = (lines[line - 1].x + gdPrevXoffset) * gdPrevScale, y1 = gdPrevHeight - (lines[line - 1].y + gdPrevYoffset) * gdPrevScale;
            float x2 = (lines[line].x + gdPrevXoffset) * gdPrevScale, y2 = gdPrevHeight - (lines[line].y + gdPrevYoffset) * gdPrevScale;
            gdImageLine(gdPrevImage, x1, y1, x2, y2, gdAntiAliased);
        }
    }
    debug_stderr("fast path duration : %.04lfs\n", get_time_double() - benchmarkStart);
    
    //info
    gdImageLine(gdPrevImage, 0, gdPrevHeight+gdPrevStrFontHeight*(gdPrevTxtLines-2)+3, gdPrevWidthFull, gdPrevHeight+gdPrevStrFontHeight*(gdPrevTxtLines-2)+3, gdPrevColorSubGrid);
    
    //color code
    char gdPrevStrGrid[64]; sprintf(gdPrevStrGrid, "[%s : %dmm] ", strGDCore[STR_GDCORE::GDCORE_GRID][language], gdPrevGrid);
    char gdPrevStrAxis[64]; sprintf(gdPrevStrAxis, "[%s] ", strGDCore[STR_GDCORE::GDCORE_AXIS][language]);
    char gdPrevStrTool[64]; sprintf(gdPrevStrTool, "[%s] ", strGDCore[STR_GDCORE::GDCORE_TOOLPATHS][language]);
    char gdPrevStrFast[64]; sprintf(gdPrevStrFast, "[%s] ", strGDCore[STR_GDCORE::GDCORE_FAST][language]);
    char gdPrevStrWork[64]; sprintf(gdPrevStrWork, "[%s] ", strGDCore[STR_GDCORE::GDCORE_LINEAR][language]);
    char gdPrevStrRadius[64]; sprintf(gdPrevStrRadius, "[%s]", strGDCore[STR_GDCORE::GDCORE_CIRCULAR][language]);
    int gdPrevStrLen = strlen(gdPrevStrGrid) + strlen(gdPrevStrAxis) + strlen(gdPrevStrTool) + strlen(gdPrevStrFast) + strlen(gdPrevStrWork) + strlen(gdPrevStrRadius);
    int gdPrevStrWidth = gdPrevStrLen * gdPrevStrFontWidth;
    int gdPrevStrX = (gdPrevWidthFull - gdPrevStrWidth) / 2;
    gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 2) + 3, (unsigned char*) gdPrevStrGrid, gdPrevColorGrid);
    gdPrevStrX += strlen(gdPrevStrGrid) * gdPrevStrFontWidth;
    gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 2) + 3, (unsigned char*) gdPrevStrAxis, gdPrevColorAxis);
    gdPrevStrX += strlen(gdPrevStrAxis) * gdPrevStrFontWidth;
    gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 2) + 3, (unsigned char*) gdPrevStrTool, gdPrevColorTool);
    gdPrevStrX += strlen(gdPrevStrTool) * gdPrevStrFontWidth;
    gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 2) + 3, (unsigned char*) gdPrevStrFast, gdPrevColorFast);
    gdPrevStrX += strlen(gdPrevStrFast) * gdPrevStrFontWidth;
    gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 2) + 3, (unsigned char*) gdPrevStrWork, gdPrevColorWork);
    gdPrevStrX += strlen(gdPrevStrWork) * gdPrevStrFontWidth;
    gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 2) + 3, (unsigned char*) gdPrevStrRadius, gdPrevColorRadius);
    
    //work time
    char timeArr[] = "0000h 00min";
    sec2charArr(timeArr, summary->timeTotal * 60); 
    sprintf(strBuffer, "%s : %s", strGDCore[STR_GDCORE::GDCORE_DURATION][language], timeArr);
    if (speedPercent != 100){
        sec2charArr(timeArr, (summary->timeTotal * 60) / ((float)speedPercent / 100.f)); 
        char strBuffer1[512]; sprintf(strBuffer1, ", %s %d%%: %s", strGDCore[STR_GDCORE::GDCORE_FEED][language], speedPercent, timeArr);
        strcat(strBuffer, strBuffer1);
    }
    gdPrevStrWidth = strlen(strBuffer) * gdPrevStrFontWidth; gdPrevStrX = (gdPrevWidthFull - gdPrevStrWidth) / 2;
    gdImageLine(gdPrevImage, 0, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 1) + 3, gdPrevWidthFull, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 1) + 3, gdPrevColorSubGrid);
    gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 1) + 3, (unsigned char*)strBuffer, gdPrevColorLimits);
    
    //limits
    //X min
    sprintf(strBuffer, " %.2lfmm ", limits->xMin);
    NNSgdImageArrow(gdPrevImage, gdPrevXminPx, gdPrevHeight+gdPrevStrFontHeight-1, gdPrevStrFontHeight, 6, 6, 3, gdPrevColorLimits, true);
    gdImageString(gdPrevImage, gdFontSmall, gdPrevXminPx+2, gdPrevHeight, (unsigned char*)strBuffer, gdPrevColorLimits);
    gdImageLine(gdPrevImage, gdPrevXminPx+1, gdPrevHeight+gdPrevStrFontHeight, gdPrevXminPx + strlen(strBuffer)*gdPrevStrFontWidth, gdPrevHeight+gdPrevStrFontHeight, gdPrevColorLimits);
    
    //X max
    sprintf(strBuffer, " %.2lfmm ", limits->xMax);
    NNSgdImageArrow(gdPrevImage, gdPrevXmaxPx, gdPrevHeight+gdPrevStrFontHeight-1, gdPrevStrFontHeight, 6, 6, 3, gdPrevColorLimits, true);
    gdImageString(gdPrevImage, gdFontSmall, gdPrevXmaxPx - strlen(strBuffer)*gdPrevStrFontWidth, gdPrevHeight, (unsigned char*)strBuffer, gdPrevColorLimits);
    gdImageLine(gdPrevImage, gdPrevXmaxPx-1, gdPrevHeight+gdPrevStrFontHeight, gdPrevXmaxPx - strlen(strBuffer)*gdPrevStrFontWidth, gdPrevHeight+gdPrevStrFontHeight, gdPrevColorLimits);
    
    //X total
    sprintf(strBuffer, " %.2lfmm ", limits->xMax - limits->xMin);
    if (numDiffFloat(limits->xMin, limits->xMax) > 0.001){
        NNSgdImageArrow(gdPrevImage, gdPrevXminPx, gdPrevHeight+gdPrevStrFontHeight*2, gdPrevStrFontHeight, 6, 6, 3, gdPrevColorLimits, true);
        NNSgdImageArrow(gdPrevImage, gdPrevXmaxPx, gdPrevHeight+gdPrevStrFontHeight*2, gdPrevStrFontHeight, 6, 6, 3, gdPrevColorLimits, true);
        gdImageLine(gdPrevImage, gdPrevXminPx+1, gdPrevHeight+gdPrevStrFontHeight*2, gdPrevXmaxPx-1, gdPrevHeight+gdPrevStrFontHeight*2, gdPrevColorLimits);
        gdImageString(gdPrevImage, gdFontSmall, (gdPrevXminPx+(gdPrevXmaxPx-gdPrevXminPx)/2)-strlen(strBuffer)*gdPrevStrFontWidth/2, gdPrevHeight+gdPrevStrFontHeight, (unsigned char*) strBuffer, gdPrevColorLimits);
    }
    
    //Y min
    sprintf(strBuffer, " %.2lfmm ", limits->yMin);
    NNSgdImageArrow(gdPrevImage, gdPrevWidth+gdPrevStrFontHeight-1, gdPrevYminPx, gdPrevStrFontHeight, 6, 6, 1, gdPrevColorLimits, true);
    gdImageStringUp(gdPrevImage, gdFontSmall, gdPrevWidth, gdPrevYminPx-2, (unsigned char*) strBuffer, gdPrevColorLimits);
    gdImageLine(gdPrevImage, gdPrevWidth+gdPrevStrFontHeight, gdPrevYminPx-1, gdPrevWidth+gdPrevStrFontHeight, gdPrevYminPx-strlen(strBuffer)*gdPrevStrFontWidth, gdPrevColorLimits);
    
    //Y max
    sprintf(strBuffer, " %.2lfmm ", limits->yMax);
    NNSgdImageArrow(gdPrevImage, gdPrevWidth+gdPrevStrFontHeight-1, gdPrevYmaxPx, gdPrevStrFontHeight, 6, 6, 1, gdPrevColorLimits, true);
    gdImageStringUp(gdPrevImage, gdFontSmall, gdPrevWidth, gdPrevYmaxPx+strlen(strBuffer)*gdPrevStrFontWidth, (unsigned char*) strBuffer, gdPrevColorLimits);
    gdImageLine(gdPrevImage, gdPrevWidth+gdPrevStrFontHeight, gdPrevYmaxPx+1, gdPrevWidth+gdPrevStrFontHeight, gdPrevYmaxPx+strlen(strBuffer)*gdPrevStrFontWidth, gdPrevColorLimits);
    
    //Y total
    sprintf(strBuffer, " %.2lfmm ", limits->yMax - limits->yMin);
    if (numDiffFloat(limits->yMin, limits->yMax) > 0.001){
        NNSgdImageArrow(gdPrevImage, gdPrevWidth+gdPrevStrFontHeight*2, gdPrevYminPx, gdPrevStrFontHeight, 6, 6, 1, gdPrevColorLimits, true);
        NNSgdImageArrow(gdPrevImage, gdPrevWidth+gdPrevStrFontHeight*2, gdPrevYmaxPx, gdPrevStrFontHeight, 6, 6, 1, gdPrevColorLimits, true);
        gdImageLine(gdPrevImage, gdPrevWidth+gdPrevStrFontHeight*2, gdPrevYminPx-1, gdPrevWidth+gdPrevStrFontHeight*2, gdPrevYmaxPx+1, gdPrevColorLimits);
        gdImageStringUp(gdPrevImage, gdFontSmall, gdPrevWidth+gdPrevStrFontHeight, (gdPrevYminPx+(gdPrevYmaxPx-gdPrevYminPx)/2)+strlen(strBuffer)*gdPrevStrFontWidth/2, (unsigned char*) strBuffer, gdPrevColorLimits);
    }
    
    //Z
    sprintf(strBuffer, "Z min : %.2lfmm <> Z max : %.2lfmm", limits->zMin, limits->zMax);
    gdPrevStrWidth = strlen(strBuffer) * gdPrevStrFontWidth; gdPrevStrX = (gdPrevWidthFull - gdPrevStrWidth) / 2;
    gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX , gdPrevHeight+gdPrevStrFontHeight*(gdPrevTxtLines-3), (unsigned char*)strBuffer, gdPrevColorLimits);
    gdImageLine(gdPrevImage, gdPrevStrX, gdPrevHeight+gdPrevStrFontHeight*(gdPrevTxtLines-2), gdPrevStrX + gdPrevStrWidth , gdPrevHeight+gdPrevStrFontHeight*(gdPrevTxtLines-2), gdPrevColorLimits);
    
    //output file
    sprintf(previewFilePath, "%s.png", file);
    gdFileHandle = fopen(previewFilePath, "wb");
    if (gdFileHandle != NULL){
        gdImagePng(gdPrevImage, gdFileHandle);
        fclose(gdFileHandle);
        NNSPNGaddPPM(previewFilePath, (unsigned int)(gdPrevScale * 1000.)); //write proper ppm
    } else {
        gdImageDestroy(gdPrevImage);
        return -ENOENT; //failed to write
    }
    gdImageDestroy(gdPrevImage);
    
    debug = debugBack;
    return 0; //ok
}



