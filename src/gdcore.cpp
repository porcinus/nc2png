/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to libGD, generate preview from a custom structs (ncparser.h/cpp).
*/

#include "gdcore.h"

 
void NNSgdImageLineDashed (gdImagePtr image, int x1, int y1, int x2, int y2, int color1, int color2, int space) { //draw dashed line the proper way, GD one is glitched
    int style [space * 2 + 1];
    for(int i=0;i<space;i++){style[i]=color1;}
    for(int i=0;i<space;i++){style[space + i]=color2;}
    gdImageSetStyle (image, style, space*2);
    gdImageLine (image, x1, y1, x2, y2, gdStyled);
}

int NNSgdImageColorAllocateFade (gdImagePtr image, int *rgbArr1, int *rgbArr2, double fade) { //proportional color gradian
    if (fade < 0.){return gdImageColorAllocate (image, rgbArr1[0], rgbArr1[1], rgbArr1[2]);}
    if (fade > 1.){return gdImageColorAllocate (image, rgbArr2[0], rgbArr2[1], rgbArr2[2]);}
    return gdImageColorAllocate (image, rgbArr1[0] + (rgbArr2[0] - rgbArr1[0]) * fade, rgbArr1[1] + (rgbArr2[1] - rgbArr1[1]) * fade, rgbArr1[2] + (rgbArr2[2] - rgbArr1[2]) * fade);
}

void NNSgdImageArcThick (gdImagePtr image, int cx, int cy, int width, int height, int start, int end, int thickness, int color, int resolution) { //draw thick arc
    if (resolution < 1) {resolution = 20;}
    if (thickness < 2) {gdImageArc (image, cx, cy, width, height, start, end, color); return;}
    double a1 = deg2rad(start), a2 = deg2rad(end), arclenght = abs(a2 - a1) * (height / 2);
    int hwidth = width/2, hheight = height/2, step = (double)ceil(arclenght / resolution) + 1;
    double stepangle = (a2 - a1) / step;
    int radiusxmin = hwidth - thickness / 2, radiusymin = hheight - thickness / 2, radiusxmax = hwidth + thickness / 2, radiusymax = hheight + thickness / 2;
    gdImageFilledEllipse (image, round(cx + cos(a1) * hwidth), round(cy + sin(a1) * hheight), thickness, thickness, color); gdImageFilledEllipse (image, round(cx + cos(a2) * hwidth), round(cy + sin(a2) * hheight), thickness, thickness, color);
    gdPoint *pointsArr = new gdPoint [(step+1) * 2 + 1];
    double tmpa, tmpx, tmpy;
    for (int i=0, j=(step*2)+1; i < step + 1 /* 2*/; i++, j--) {
        tmpa = a1 + (stepangle * i); tmpx =  cos(tmpa); tmpy = sin(tmpa);
        pointsArr[i].x = cx + tmpx * radiusxmin; pointsArr[i].y = cy + tmpy * radiusymin;
        pointsArr[j].x = cx + tmpx * radiusxmax; pointsArr[j].y = cy + tmpy * radiusymax;
    }
    gdImageFilledPolygon (image, pointsArr, (step+1)*2, color); delete pointsArr;
}

void NNSgdImageLineThick (gdImagePtr image, int x1, int y1, int x2, int y2, int thickness, int color) { //draw thick line
    if (thickness < 2) {gdImageLine (image, x1, y1, x2, y2, color); return;}
    gdImageFilledEllipse (image, x1, y1, thickness, thickness, color); gdImageFilledEllipse (image, x2, y2, thickness, thickness, color);
    int halfthick = thickness / 2;
    if (x1==x2) {gdImageFilledRectangle (image, x1 - halfthick, y1, x2 + halfthick, y2, color); return;
    } else if (y1==y2) {gdImageFilledRectangle (image, x1, y1 - halfthick, x2, y2 + halfthick, color); return;
    } else {
        double angle = atan((y2 - y1) / (x2 - x1));
        double deccos = halfthick * cos(angle);
        double decsin = halfthick * sin(angle);
        gdPoint *pointsArr = new gdPoint [5];
        pointsArr[0].x = round(x1 + decsin); pointsArr[0].y = round(y1 - deccos);
        pointsArr[1].x = round(x1 - decsin); pointsArr[1].y = round(y1 + deccos);
        pointsArr[2].x = round(x2 - decsin); pointsArr[2].y = round(y2 + deccos);
        pointsArr[3].x = round(x2 + decsin); pointsArr[3].y = round(y2 - deccos);
        gdImageFilledPolygon (image, pointsArr, 4, color); delete pointsArr;
    }
}

void NNSgdImageLineThickV (gdImagePtr image, int x1, int y1, int x2, int y2, double d1, double d2, int color) { //draw thick line with different start and end thickness
    if(x2 - x1 <= 0){int tmpx = x1, tmpy = y1, tmpd = d1; x1 = x2; y1 = y2; x2 = tmpx; y2 = tmpy; d1 = d2; d2 = tmpd;}
    gdImageFilledEllipse (image, x1, y1, d1, d1, color); gdImageFilledEllipse (image, x2, y2, d2, d2, color);
    int hd1 = d1/2, hd2 = d2/2, vx = x2- x1, vy = y2 - y1;
    if (y2 == y1) {y2++;} if (x2 == x1) {x2++;}
    double centerdist = sqrt (vx * vx + vy * vy), relangle = acos ((double)(hd1 - hd2) / centerdist), absangle = atan ((double)vy / vx);
    double anglea = relangle - absangle, angleb = relangle + absangle;
    double sina = sin(anglea), sinb = sin(angleb), cosa = cos(anglea), cosb = cos(angleb);
    gdPoint *pointsArr = new gdPoint [5];
    pointsArr[0].x = round(x1 + hd1 * cosa); pointsArr[0].y = round(y1 - hd1 * sina);
    pointsArr[1].x = round(x2 + hd2 * cosa); pointsArr[1].y = round(y2 - hd2 * sina);
    pointsArr[2].x = round(x2 + hd2 * cosb); pointsArr[2].y = round(y2 + hd2 * sinb);
    pointsArr[3].x = round(x1 + hd1 * cosb); pointsArr[3].y = round(y1 + hd1 * sinb);
    gdImageFilledPolygon (image, pointsArr, 4, color); delete pointsArr;

}

void NNSgdImageArcThickV (gdImagePtr image, int cx, int cy, int width, int height, int start, int end, double d1, double d2, int color, int resolution) { //draw thick arc with different start and end thickness
    if (resolution < 1) {resolution = 20;}
    if (d1 < 2.) {gdImageArc (image, cx, cy, width, height, start, end, color); return;}
    double a1 = deg2rad(start), a2 = deg2rad(end), arclenght = abs(a2 - a1) * (height / 2);
    int hwidth = width/2, hheight = height/2, step = ceil(arclenght / resolution) + 1; 
    gdImageFilledEllipse (image, round(cx + cos(a1) * hwidth), round(cy + sin(a1) * hheight), d1, d1, color);
    gdImageFilledEllipse (image, round(cx + cos(a2) * hwidth), round(cy + sin(a2) * hheight), d2, d2, color);
    gdPoint *pointsArr = new gdPoint [(step + 1) * 2 + 1];
    double tmpa, tmpx, tmpy, stepangle = (a2 - a1) / step, stepd = (d2 - d1) / step;
    int radiusxmin, radiusymin, radiusxmax, radiusymax;
    for (int i=0, j=(step*2)+1; i < step + 1; i++, j--) {
        tmpa = a1 + (stepangle * i);
        tmpx =  cos(tmpa); tmpy = sin(tmpa);
        tmpa = (d1 + stepd * i) / 2;
        radiusxmin = hwidth - tmpa; radiusymin = hheight - tmpa; radiusxmax = hwidth + tmpa; radiusymax = hheight + tmpa;
        pointsArr[i].x = cx + tmpx * radiusxmin; pointsArr[i].y = cy + tmpy * radiusymin;
        pointsArr[j].x = cx + tmpx * radiusxmax; pointsArr[j].y = cy + tmpy * radiusymax;
    }
    gdImageFilledPolygon (image, pointsArr, (step+1)*2, color); delete pointsArr;
}

void NNSgdImageArrow (gdImagePtr image, int x, int y, int lenght, int width, int height, int dir, int color, bool filled) { //draw simple arrow
    gdPoint *pointsArr = new gdPoint [5];
    if (dir == 0) { //x+
        gdImageLine (image, x, y, x + lenght, y, color);
        pointsArr[0].x = x+lenght-width; pointsArr[0].y = y-height/2;
        pointsArr[1].x = x+lenght; pointsArr[1].y = y;
        pointsArr[2].x = x+lenght-width; pointsArr[2].y = y+height/2;
    } else if (dir == 1) { //x-
        gdImageLine (image, x, y, x - lenght, y, color);
        pointsArr[0].x = x-lenght+width; pointsArr[0].y = y-height/2;
        pointsArr[1].x = x-lenght; pointsArr[1].y = y;
        pointsArr[2].x = x-lenght+width; pointsArr[2].y = y+height/2;
    } else if (dir == 2) { //y+
        gdImageLine (image, x, y, x, y + lenght, color);
        pointsArr[0].x = x-height/2; pointsArr[0].y = y+lenght-width;
        pointsArr[1].x = x; pointsArr[1].y = y+lenght;
        pointsArr[2].x = x+height/2; pointsArr[2].y = y+lenght-width;
    } else if (dir == 3) { //y-
        gdImageLine (image, x, y, x, y - lenght, color);
        pointsArr[0].x = x-height/2; pointsArr[0].y = y-lenght+width;
        pointsArr[1].x = x; pointsArr[1].y = y-lenght;
        pointsArr[2].x = x+height/2; pointsArr[2].y = y-lenght+width;
    } else {delete pointsArr; return;}
    if (filled) {gdImageFilledPolygon (image, pointsArr, 3, color);} else {gdImagePolygon (image, pointsArr, 3, color);}
    delete pointsArr;
}

void NNSgdImageArc (gdImagePtr image, int cx, int cy, int width, int height, int start, int end, int color, int resolution) { //draw arc, allow precision definition, GD one draw one line per degree
    if (resolution < 1) {resolution = 20;}
    double a1 = deg2rad(start), a2 = deg2rad(end), arclenght = abs(a2 - a1) * (height / 2);
    int hwidth = width/2, hheight = height/2, step = (double)ceil(arclenght / resolution) + 1;
    double stepangle = (a2 - a1) / step;
    int lastx = cx + cos(a1) * hwidth, lasty = cy + sin(a1) * hheight, x, y;
    for (int i=1; i < step + 1; i++) {
        x = cx + cos(a1 + stepangle * i) * hwidth; y = cy + sin(a1 + stepangle * i) * hheight;
        gdImageLine (image, x, y, lastx, lasty, color); lastx = x; lasty = y;
    }
}

void sec2charArr (char *arr, double time) { //convert seconds to char array in format : XXsec / XXmin / XXh XXmin
    int hours = 0, minutes = 0, seconds = 0;
    seconds = floor (time);
    if (seconds < 60) {sprintf(arr, "%dsec", seconds);
    } else if (seconds < 3600) {minutes = ceil (seconds / 60); sprintf(arr, "%dmin", minutes);
    }else {
        hours = floor (seconds / 3600);
        minutes = ceil ((seconds%3600) / 60);
        if (minutes > 59) {minutes = 0; hours++;}
        sprintf(arr, "%02dh %02dmin", hours, minutes);
    }
}

int NNSPNGaddPPM (char *file, unsigned int ppm) { //write specific PPM to PNG file, can be done via libpng but it is faster that way
    FILE* tmpHandle = fopen(file, "rb+"); //file stream
    if (tmpHandle != NULL) { //valid file stream
        char tmpBuffer[5]; //signature read buffer
        if (fgets (tmpBuffer , 5 , tmpHandle) != NULL){if (strcmp(tmpBuffer, "\x89PNG\0") != 0){fclose(tmpHandle); return 0;} //security, not png signature
        } else {fclose(tmpHandle); return 0;} //something failed when trying to read signature
        char pHYsIdent[] = "pHYs"; char currChar; unsigned int identPos = 0, seek = 5;
        fseek (tmpHandle, seek , SEEK_SET); //seek after signature
        do {
            seek++; currChar = (char) fgetc (tmpHandle);
            if ((char)currChar == (char)pHYsIdent[identPos]) {identPos++;} else {identPos = 0;}
            if (currChar == EOF || seek > 255) {fclose(tmpHandle); return 0;} //failed to found pHYs block after 255 bytes
        } while (identPos < 4);
        fseek (tmpHandle, seek-4 , SEEK_SET); //seek before pHYs block start

        char tmpBufferPPM[14] = {'p','H','Y','s','\0','\0','\0','\0','\0','\0','\0','\0','\1'}; //pHYs block w/o crc32

        char *tmpPtr = (char*) &ppm; //ppm pointer to swap endian
        tmpBufferPPM[4] = tmpPtr[3]; tmpBufferPPM[5] = tmpPtr[2]; tmpBufferPPM[6] = tmpPtr[1]; tmpBufferPPM[7] = tmpPtr[0]; //swap endian x ppm
        tmpBufferPPM[8] = tmpPtr[3]; tmpBufferPPM[9] = tmpPtr[2]; tmpBufferPPM[10] = tmpPtr[1]; tmpBufferPPM[11] = tmpPtr[0]; //swap endian y ppm
        fwrite(&tmpBufferPPM, 1, 13, tmpHandle); //write ppm block

        unsigned long crc = crc32 (0, (Bytef*) &tmpBufferPPM, 13); //use crc32 funct from zlib
        tmpPtr = (char*) &crc; tmpBufferPPM[0] = tmpPtr[3]; tmpBufferPPM[1] = tmpPtr[2]; tmpBufferPPM[2] = tmpPtr[1]; tmpBufferPPM[3] = tmpPtr[0]; //swap endian
        fwrite(&tmpBufferPPM, 1, 4, tmpHandle); //write crc

        fclose(tmpHandle); //close stream
        return 1; //success
    }
    return 0; //failed
}

int gdPreview (char *file, int gdPrevWidth, int gdPrevArcRes, ncFlagsStruc *flags, ncLineStruc *lines, ncToolStruc *tools, ncDistTimeStruc *ops, ncLimitStruc *limits, ncLinesCountStruc *linescount, bool debugOutput) { //generate image preview off ncparser data
    FILE *gdFileHandle; //file handle

    char previewFilePath [PATH_MAX]; //full path to nc file

    int gdPrevHeight = gdPrevWidth;
    int gdPrevAxisFontWidth = gdFontGiant->w, gdPrevAxisFontHeight = gdFontGiant->h; //px
    int gdPrevStrFontWidth = gdFontSmall->w, gdPrevStrFontHeight = gdFontSmall->h; //px

    int gdPrevXmin = 0, gdPrevXmax = 0, gdPrevYmin = 0, gdPrevYmax = 0, gdPrevMargin = 10, gdPrevGrid = 10, gdPrevSubGrid = 1, tmpInt = 0; //mm

    double gdPrevScale = 1.;

    if (0. > limits[0].xMin) {gdPrevXmin = limits[0].xMin;}
    if (0. < limits[0].xMax) {gdPrevXmax = limits[0].xMax;}
    if (0. > limits[0].yMin) {gdPrevYmin = limits[0].yMin;}
    if (0. < limits[0].yMax) {gdPrevYmax = limits[0].yMax;}

    if (abs (gdPrevXmax - gdPrevXmin) > abs (gdPrevYmax - gdPrevYmin)) { //compute height, width > height
        gdPrevHeight = gdPrevWidth / (double)((abs (gdPrevXmax - gdPrevXmin) + gdPrevMargin * 2) / (double)(abs (gdPrevYmax - gdPrevYmin) + gdPrevMargin * 2));
        gdPrevScale = gdPrevWidth / (double)(abs (gdPrevXmax - gdPrevXmin) + gdPrevMargin * 2);
    } else { //height > width
        gdPrevHeight = gdPrevWidth;
        gdPrevScale = gdPrevWidth / (double)(abs (gdPrevYmax - gdPrevYmin) + gdPrevMargin * 2);
    }

	int gdPrevXoffset = abs (gdPrevXmin) + gdPrevMargin; int gdPrevYoffset = abs (gdPrevYmin) + gdPrevMargin;
    int gdPrevWidthFull = gdPrevWidth + (gdPrevStrFontWidth + 10) * 2;
    int gdPrevTxtLines = 6; if (linescount[0].tools > 0) {gdPrevTxtLines++;}

    gdImagePtr gdPrevImage = gdImageCreateTrueColor (gdPrevWidthFull, gdPrevHeight + gdPrevStrFontHeight * gdPrevTxtLines + 4);

    if (gdPrevImage != NULL) {
        if (linescount[0].tools > 0) {gdPrevTxtLines--;}

        //couleur gd : ok+opt
        int gdPrevColorBackground = gdImageColorAllocate (gdPrevImage, 0, 0, 0); //couleur du fond
        int gdPrevColorAxis = gdImageColorAllocate (gdPrevImage, 180, 180, 180); //couleur des axes
        int gdPrevColorGrid = gdImageColorAllocate (gdPrevImage, 70, 70, 70); //couleur des graduations
        int gdPrevColorSubGrid = gdImageColorAllocate (gdPrevImage, 35, 35, 35); //couleur des graduations
        int gdPrevColorWork = gdImageColorAllocate (gdPrevImage, 0, 0, 255); //couleur des trajectoires en mode travail
        int gdPrevColorRadius = gdImageColorAllocate (gdPrevImage, 0, 255, 0); //couleur des trajectoires en mode circulaire
        int gdPrevColorFast = gdImageColorAllocate (gdPrevImage, 255, 0, 0); //couleur des trajectoires en mode rapide
        int gdPrevColorToolLowerArr[] = {0, 80, 80}; //couleur des outils
        int gdPrevColorToolHigherArr[] = {0, 200, 200}; //couleur des outils
        int gdPrevColorTool = gdImageColorAllocate (gdPrevImage, gdPrevColorToolHigherArr[0], gdPrevColorToolHigherArr[1], gdPrevColorToolHigherArr[2]); //couleur des outils
        int gdPrevColorToolStr = gdImageColorAllocate (gdPrevImage, gdPrevColorToolHigherArr[0], gdPrevColorToolHigherArr[1], gdPrevColorToolHigherArr[2]); //couleur des outils
        int gdPrevColorLimits = gdImageColorAllocate (gdPrevImage, 255, 255, 0); //couleur des limites
        int gdPrevColorLimitsLines = gdImageColorAllocate (gdPrevImage, 180, 180, 0); //couleur des lignes des limites

        //reset background image : ok+opt
        gdImageFill (gdPrevImage, 0, 0, gdPrevColorBackground);

        //divers : ok+opt
        int gdPrevOrigX = gdPrevXoffset * gdPrevScale; //origne x px
        int gdPrevOrigY = gdPrevHeight - gdPrevYoffset * gdPrevScale; //origne y px

        //dessin sub grille : ok+opt !double needed because grid shift with int
        double gdPrevSubGridScaled = gdPrevSubGrid * gdPrevScale;
        if(gdPrevSubGridScaled > 5.){
            for(double gdPrevX1 = gdPrevOrigX; gdPrevX1 < gdPrevWidth; gdPrevX1 += gdPrevSubGridScaled){gdImageLine(gdPrevImage,gdPrevX1,0,gdPrevX1,gdPrevHeight,gdPrevColorSubGrid);} //boucle dessin grille y+
            for(double gdPrevX1 = gdPrevOrigX; gdPrevX1 > 0; gdPrevX1 -= gdPrevSubGridScaled){gdImageLine(gdPrevImage,gdPrevX1,0,gdPrevX1,gdPrevHeight,gdPrevColorSubGrid);} //boucle dessin grille y-
            for(double gdPrevY1 = gdPrevOrigY; gdPrevY1 < gdPrevHeight; gdPrevY1 += gdPrevSubGridScaled){gdImageLine(gdPrevImage,0,gdPrevY1,gdPrevWidth,gdPrevY1,gdPrevColorSubGrid);} //boucle dessin grille x+
            for(double gdPrevY1 = gdPrevOrigY; gdPrevY1 > 0; gdPrevY1 -= gdPrevSubGridScaled){gdImageLine(gdPrevImage,0,gdPrevY1,gdPrevWidth,gdPrevY1,gdPrevColorSubGrid);} //boucle dessin grille x-
        }
        
        //dessin grille : ok+opt !double needed because grid shift with int
        double gdPrevGrid_scaled = gdPrevGrid * gdPrevScale;
        for(double gdPrevX1=gdPrevOrigX;gdPrevX1<gdPrevWidth;gdPrevX1+=gdPrevGrid_scaled){gdImageLine(gdPrevImage,gdPrevX1,0,gdPrevX1,gdPrevHeight,gdPrevColorGrid);} //boucle dessin grille y+
        for(double gdPrevX1=gdPrevOrigX;gdPrevX1>0;gdPrevX1-=gdPrevGrid_scaled){gdImageLine(gdPrevImage,gdPrevX1,0,gdPrevX1,gdPrevHeight,gdPrevColorGrid);} //boucle dessin grille y-
        for(double gdPrevY1=gdPrevOrigY;gdPrevY1<gdPrevHeight;gdPrevY1+=gdPrevGrid_scaled){gdImageLine(gdPrevImage,0,gdPrevY1,gdPrevWidth,gdPrevY1,gdPrevColorGrid);} //boucle dessin grille x+
        for(double gdPrevY1=gdPrevOrigY;gdPrevY1>0;gdPrevY1-=gdPrevGrid_scaled){gdImageLine(gdPrevImage,0,gdPrevY1,gdPrevWidth,gdPrevY1,gdPrevColorGrid);} //boucle dessin grille x-
        
        //rectangle pointille des limites : ok+opt
        int gdPrevXminPx = (limits[0].xMin + gdPrevXoffset)*gdPrevScale;
        int gdPrevXmaxPx = (limits[0].xMax + gdPrevXoffset)*gdPrevScale;
        int gdPrevYminPx = gdPrevHeight-(limits[0].yMin + gdPrevYoffset)*gdPrevScale;
        int gdPrevYmaxPx = gdPrevHeight-(limits[0].yMax + gdPrevYoffset)*gdPrevScale;

        NNSgdImageLineDashed(gdPrevImage,gdPrevXminPx,gdPrevYmaxPx,gdPrevXminPx,gdPrevHeight,gdPrevColorLimitsLines,gdTransparent,4);
        NNSgdImageLineDashed(gdPrevImage,gdPrevXmaxPx,gdPrevYmaxPx,gdPrevXmaxPx,gdPrevHeight,gdPrevColorLimitsLines,gdTransparent,4);
        NNSgdImageLineDashed(gdPrevImage,gdPrevXminPx,gdPrevYminPx,gdPrevWidth,gdPrevYminPx,gdPrevColorLimitsLines,gdTransparent,4);
        NNSgdImageLineDashed(gdPrevImage,gdPrevXminPx,gdPrevYmaxPx,gdPrevWidth,gdPrevYmaxPx,gdPrevColorLimitsLines,gdTransparent,4);
        
        //dessin tracees d'outils : ok+opt
        int line = 0, g;
        if(linescount[0].tools > 0){
            double toolDia = 1, /*toolAngle = 0, */dz = .0, lastdZ = .0;
            while (lines[line].g != -1) { //boucle de dessin outil
                g = lines[line].g;
                if (lines[line].tool != -1 && line != 0){
                    toolDia = tools[lines[line].tool].diameter * gdPrevScale;
                    if (numDiffDouble(limits[0].zMaxWork, limits[0].zMinWork) > 0.01) {gdPrevColorTool = NNSgdImageColorAllocateFade(gdPrevImage, gdPrevColorToolLowerArr, gdPrevColorToolHigherArr, (lines[line].z - limits[0].zMinWork) / (limits[0].zMaxWork - limits[0].zMinWork));
                    } else {gdPrevColorTool = gdImageColorAllocate (gdPrevImage, gdPrevColorToolLowerArr[0], gdPrevColorToolLowerArr[1], gdPrevColorToolLowerArr[2]);}
                    
                    if (tools[lines[line].tool].angle <= 0.01) {
                        if (g == 2 || g == 3) {
                            if (numDiffDouble (lines[line].startAngle, lines[line].endAngle) > 1.) {NNSgdImageArcThick(gdPrevImage, (lines[line].i + gdPrevXoffset) * gdPrevScale, gdPrevHeight - (lines[line].j + gdPrevYoffset) * gdPrevScale, lines[line].radius * gdPrevScale * 2, lines[line].radius * gdPrevScale * 2, lines[line].startAngle, lines[line].endAngle, toolDia, gdPrevColorTool, gdPrevArcRes);}
                            if (numDiffDouble (lines[line].startAngle1, lines[line].endAngle1) > 1.) {NNSgdImageArcThick(gdPrevImage, (lines[line].i + gdPrevXoffset) * gdPrevScale, gdPrevHeight - (lines[line].j + gdPrevYoffset) * gdPrevScale, lines[line].radius * gdPrevScale * 2, lines[line].radius * gdPrevScale * 2, lines[line].startAngle1, lines[line].endAngle1, toolDia, gdPrevColorTool, gdPrevArcRes);}
                        } else if (g == 81 || g == 82 || g == 83) {gdImageFilledEllipse (gdPrevImage, (lines[line].x + gdPrevXoffset) * gdPrevScale, gdPrevHeight - (lines[line].y + gdPrevYoffset) * gdPrevScale, toolDia, toolDia, gdPrevColorTool);
                        } else if (g == 1) {NNSgdImageLineThick (gdPrevImage,(lines[line-1].x + gdPrevXoffset) * gdPrevScale, gdPrevHeight - (lines[line-1].y + gdPrevYoffset) * gdPrevScale,(lines[line].x + gdPrevXoffset) * gdPrevScale, gdPrevHeight - (lines[line].y + gdPrevYoffset) * gdPrevScale, toolDia, gdPrevColorTool);}
                    } else {
                        dz = (tan (deg2rad (tools[lines[line].tool].angle)) * abs (lines[line].z)) * gdPrevScale * 2; /*if (dz < 1.) {dz = 1.;};*/ if(dz > toolDia * 2){dz = toolDia * 2;}
                        if (g == 2) {
                            if (numDiffDouble (lines[line].startAngle, lines[line].endAngle) > 1.) {NNSgdImageArcThickV(gdPrevImage,(lines[line].i+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line].j+gdPrevYoffset)*gdPrevScale,lines[line].radius*gdPrevScale*2,lines[line].radius*gdPrevScale*2,lines[line].startAngle,lines[line].endAngle,lastdZ,dz,gdPrevColorTool,gdPrevArcRes);}
                            if (numDiffDouble (lines[line].startAngle1, lines[line].endAngle1) > 1.) {NNSgdImageArcThickV(gdPrevImage,(lines[line].i+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line].j+gdPrevYoffset)*gdPrevScale,lines[line].radius*gdPrevScale*2,lines[line].radius*gdPrevScale*2,lines[line].startAngle1,lines[line].endAngle1,lastdZ,dz,gdPrevColorTool,gdPrevArcRes);}
                        } if (g == 3) {
                            if (numDiffDouble (lines[line].startAngle, lines[line].endAngle) > 1.) {NNSgdImageArcThickV(gdPrevImage,(lines[line].i+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line].j+gdPrevYoffset)*gdPrevScale,lines[line].radius*gdPrevScale*2,lines[line].radius*gdPrevScale*2,lines[line].startAngle,lines[line].endAngle,dz,lastdZ,gdPrevColorTool,gdPrevArcRes);}
                            if (numDiffDouble (lines[line].startAngle1, lines[line].endAngle1) > 1.) {NNSgdImageArcThickV(gdPrevImage,(lines[line].i+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line].j+gdPrevYoffset)*gdPrevScale,lines[line].radius*gdPrevScale*2,lines[line].radius*gdPrevScale*2,lines[line].startAngle1,lines[line].endAngle1,dz,lastdZ,gdPrevColorTool,gdPrevArcRes);}
                        } else if (g == 81 || g == 82 || g == 83) {gdImageFilledEllipse (gdPrevImage, (lines[line].x + gdPrevXoffset) * gdPrevScale, gdPrevHeight - (lines[line].y + gdPrevYoffset) * gdPrevScale, toolDia, toolDia, gdPrevColorTool);
                        } else if (g == 1) {NNSgdImageLineThickV(gdPrevImage,(lines[line-1].x+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line-1].y+gdPrevYoffset)*gdPrevScale,(lines[line].x+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line].y+gdPrevYoffset)*gdPrevScale,lastdZ,dz,gdPrevColorTool);}
                        lastdZ = dz;
                    }
                }
                line++;
            }

            char gdPrevToolStr [1024];
            sprintf (gdPrevToolStr, "%s(s) : ", strGD[language][STR_GD::TOOL]);
            for (int i=0; i<100; i++) {
                if (tools[i].diameter > 0.) {
                    if (tools[i].angle > 0.) {sprintf (gdPrevToolStr, "%s [\x01 %.2fmm V%.1f\xB0]", gdPrevToolStr, tools[i].diameter, tools[i].angle);
                    } else {sprintf (gdPrevToolStr, "%s [\x01 %.2fmm]", gdPrevToolStr, tools[i].diameter);}
                }
            }

            gdImageLine(gdPrevImage,0,gdPrevHeight+gdPrevStrFontHeight*gdPrevTxtLines+3,gdPrevWidthFull,gdPrevHeight+gdPrevStrFontHeight*gdPrevTxtLines+3,gdPrevColorSubGrid);
            int gdPrevToolStrWidth = strlen (gdPrevToolStr) * gdPrevStrFontWidth, gdPrevToolStrX = (gdPrevWidthFull - gdPrevToolStrWidth) / 2;
            gdImageString(gdPrevImage, gdFontSmall, gdPrevToolStrX, gdPrevHeight+gdPrevStrFontHeight*gdPrevTxtLines+3, (unsigned char*)gdPrevToolStr, gdPrevColorToolStr);
        }
        
        //dessin origine + trait d'axe : ok+opt
        gdImageLine(gdPrevImage,0,gdPrevOrigY,gdPrevWidth-gdPrevAxisFontWidth-2,gdPrevOrigY,gdPrevColorAxis); //dessin axe x
        gdImageLine(gdPrevImage,gdPrevOrigX,gdPrevAxisFontHeight+2,gdPrevOrigX,gdPrevHeight,gdPrevColorAxis); //dessin axe y
        NNSgdImageArrow(gdPrevImage,gdPrevWidth-gdPrevAxisFontWidth-2,gdPrevOrigY,0,10,8,0,gdPrevColorAxis,true); //dessin fleche x
        gdImageFilledRectangle(gdPrevImage,gdPrevWidth-gdPrevAxisFontWidth-1, gdPrevOrigY-gdPrevAxisFontHeight/2, gdPrevWidth, gdPrevOrigY+gdPrevAxisFontHeight/2, gdPrevColorBackground); //background cleanup
        gdImageString(gdPrevImage,gdFontGiant,gdPrevWidth-gdPrevAxisFontWidth,gdPrevOrigY-gdPrevAxisFontHeight/2,(unsigned char*)"X",gdPrevColorAxis); //dessin text
        NNSgdImageArrow(gdPrevImage,gdPrevOrigX,gdPrevAxisFontHeight+2,0,10,8,3,gdPrevColorAxis,true); //dessin fleche y
        gdImageFilledRectangle(gdPrevImage,gdPrevOrigX-gdPrevAxisFontWidth/2-1, 0, gdPrevOrigX+gdPrevAxisFontWidth/2+1, gdPrevAxisFontHeight, gdPrevColorBackground); //background cleanup
        gdImageString(gdPrevImage,gdFontGiant,gdPrevOrigX-gdPrevAxisFontWidth/2+1,0,(unsigned char*)"Y",gdPrevColorAxis); //dessin text
        gdImageEllipse(gdPrevImage,gdPrevOrigX,gdPrevOrigY,15,15,gdPrevColorAxis); gdImageFilledArc(gdPrevImage,gdPrevOrigX,gdPrevOrigY,15,15,-90,0,gdPrevColorAxis,gdPie); gdImageFilledArc(gdPrevImage,gdPrevOrigX,gdPrevOrigY,15,15,90,180,gdPrevColorAxis,gdPie); //dessin origine
        
        //watermark
        sprintf (previewFilePath, "NC2PNG v%s", programversion);
        tmpInt = (strlen(previewFilePath) * gdPrevStrFontWidth) / 2;
        gdImageFilledRectangle(gdPrevImage,gdPrevWidth/2-tmpInt-2, gdPrevHeight-gdPrevStrFontHeight, gdPrevWidth/2+tmpInt+1, gdPrevHeight, gdPrevColorBackground); //background cleanup
        gdImageString(gdPrevImage, gdFontSmall, gdPrevWidth/2-tmpInt, gdPrevHeight-gdPrevStrFontHeight, (unsigned char*) previewFilePath, gdPrevColorGrid);
        
        //boucle de dessin travail : ok
        line = 0;
        while (lines[line].g != -1) {
            g = lines[line].g;
            if (g != 0 && line != 0){
                if (g == 2 || g == 3) {
                    gdImageSetAntiAliased (gdPrevImage, gdPrevColorRadius);
                    if (numDiffDouble (lines[line].startAngle, lines[line].endAngle) > 1.) {NNSgdImageArc(gdPrevImage,(lines[line].i+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line].j+gdPrevYoffset)*gdPrevScale,lines[line].radius*gdPrevScale*2,lines[line].radius*gdPrevScale*2,lines[line].startAngle,lines[line].endAngle,gdAntiAliased,gdPrevArcRes);}
                    if (numDiffDouble (lines[line].startAngle1, lines[line].endAngle1) > 1.) {NNSgdImageArc(gdPrevImage,(lines[line].i+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line].j+gdPrevYoffset)*gdPrevScale,lines[line].radius*gdPrevScale*2,lines[line].radius*gdPrevScale*2,lines[line].startAngle1,lines[line].endAngle1,gdAntiAliased,gdPrevArcRes);}
                } else if (g == 81 || g == 82 || g == 83) {gdImageSetAntiAliased (gdPrevImage, gdPrevColorFast); gdImageLine(gdPrevImage,(lines[line-1].x+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line-1].y+gdPrevYoffset)*gdPrevScale,(lines[line].x+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line].y+gdPrevYoffset)*gdPrevScale,gdAntiAliased);
                } else {gdImageSetAntiAliased (gdPrevImage, gdPrevColorWork); gdImageLine(gdPrevImage,(lines[line-1].x+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line-1].y+gdPrevYoffset)*gdPrevScale,(lines[line].x+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line].y+gdPrevYoffset)*gdPrevScale,gdAntiAliased);}
            }
            line++;
        }

        //boucle de dessin rapide : ok
        line = 0;
        gdImageSetAntiAliased (gdPrevImage, gdPrevColorFast);
        while (lines[line].g != -1) {
            if (lines[line].g == 0 && line != 0){gdImageLine(gdPrevImage,(lines[line-1].x+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line-1].y+gdPrevYoffset)*gdPrevScale,(lines[line].x+gdPrevXoffset)*gdPrevScale,gdPrevHeight-(lines[line].y+gdPrevYoffset)*gdPrevScale,gdAntiAliased);}
            line++;
        }

        //info : ok
        gdImageLine(gdPrevImage,0,gdPrevHeight+gdPrevStrFontHeight*(gdPrevTxtLines-2)+3,gdPrevWidthFull,gdPrevHeight+gdPrevStrFontHeight*(gdPrevTxtLines-2)+3,gdPrevColorSubGrid);
        char gdPrevStrGrid [64]; sprintf (gdPrevStrGrid, "[%s : %dmm] ", strGD[language][STR_GD::GRID], gdPrevGrid);
        char gdPrevStrAxis [64]; sprintf (gdPrevStrAxis, "[%s] ", strGD[language][STR_GD::AXIS]);
        char gdPrevStrTool [64]; sprintf (gdPrevStrTool, "[%s] ", strGD[language][STR_GD::TOOLPATHS]);
        char gdPrevStrFast [64]; sprintf (gdPrevStrFast, "[%s] ", strGD[language][STR_GD::FAST]);
        char gdPrevStrWork [64]; sprintf (gdPrevStrWork, "[%s] ", strGD[language][STR_GD::LINEAR]);
        char gdPrevStrRadius [64]; sprintf (gdPrevStrRadius, "[%s]", strGD[language][STR_GD::CIRCULAR]);
        int gdPrevStrLen = strlen(gdPrevStrGrid) + strlen(gdPrevStrAxis) + strlen(gdPrevStrTool) + strlen(gdPrevStrFast) + strlen(gdPrevStrWork) + strlen(gdPrevStrRadius);
        int gdPrevStrWidth = gdPrevStrLen * gdPrevStrFontWidth, gdPrevStrX = (gdPrevWidthFull - gdPrevStrWidth) / 2;
        gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 2) + 3, (unsigned char*) gdPrevStrGrid, gdPrevColorGrid); gdPrevStrX += strlen(gdPrevStrGrid) * gdPrevStrFontWidth;
        gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 2) + 3, (unsigned char*) gdPrevStrAxis, gdPrevColorAxis); gdPrevStrX += strlen(gdPrevStrAxis) * gdPrevStrFontWidth;
        gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 2) + 3, (unsigned char*) gdPrevStrTool, gdPrevColorTool); gdPrevStrX += strlen(gdPrevStrTool) * gdPrevStrFontWidth;
        gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 2) + 3, (unsigned char*) gdPrevStrFast, gdPrevColorFast); gdPrevStrX += strlen(gdPrevStrFast) * gdPrevStrFontWidth;
        gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 2) + 3, (unsigned char*) gdPrevStrWork, gdPrevColorWork); gdPrevStrX += strlen(gdPrevStrWork) * gdPrevStrFontWidth;
        gdImageString(gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 2) + 3, (unsigned char*) gdPrevStrRadius, gdPrevColorRadius);

        //temps : ok
        char gdPrevStrTime [1024], gdPrevStrTimePercent [512];
        sec2charArr (gdPrevStrTimePercent, (linescount[0].totalTimeWork + linescount[0].totalTimeFast) * 60); 
        sprintf (gdPrevStrTime, "%s : %s", strGD[language][STR_GD::DURATION], gdPrevStrTimePercent);
        if (speedPercent != 100) {
            sec2charArr (gdPrevStrTimePercent, ((linescount[0].totalTimeWork + linescount[0].totalTimeFast) * 60) / ((double)speedPercent / 100)); 
            sprintf (gdPrevStrTime, "%s, %s %d%%: %s", gdPrevStrTime, strGD[language][STR_GD::FEED], speedPercent, gdPrevStrTimePercent);
        }
        gdPrevStrWidth = strlen(gdPrevStrTime) * gdPrevStrFontWidth; gdPrevStrX = (gdPrevWidthFull - gdPrevStrWidth) / 2;
        gdImageLine (gdPrevImage,0,gdPrevHeight+gdPrevStrFontHeight*(gdPrevTxtLines-1)+3,gdPrevWidthFull,gdPrevHeight+gdPrevStrFontHeight*(gdPrevTxtLines-1)+3,gdPrevColorSubGrid);
        gdImageString (gdPrevImage, gdFontSmall, gdPrevStrX, gdPrevHeight + gdPrevStrFontHeight * (gdPrevTxtLines - 1) + 3, (unsigned char*) gdPrevStrTime, gdPrevColorLimits);
        
        //limites
        //X min
        char gdPrevStrXmin [14];
        sprintf (gdPrevStrXmin, " %.2lfmm ", limits[0].xMin);
        NNSgdImageArrow(gdPrevImage,gdPrevXminPx,gdPrevHeight+gdPrevStrFontHeight-1,gdPrevStrFontHeight,6,6,3,gdPrevColorLimits,true);
        gdImageString(gdPrevImage,gdFontSmall,gdPrevXminPx+2,gdPrevHeight,(unsigned char*) gdPrevStrXmin,gdPrevColorLimits);
        gdImageLine(gdPrevImage,gdPrevXminPx+1,gdPrevHeight+gdPrevStrFontHeight,gdPrevXminPx+strlen(gdPrevStrXmin)*gdPrevStrFontWidth,gdPrevHeight+gdPrevStrFontHeight,gdPrevColorLimits);

        //X max
        char gdPrevStrXmax [14];
        sprintf (gdPrevStrXmax, " %.2lfmm ", limits[0].xMax);
        NNSgdImageArrow(gdPrevImage,gdPrevXmaxPx,gdPrevHeight+gdPrevStrFontHeight-1,gdPrevStrFontHeight,6,6,3,gdPrevColorLimits,true);
        gdImageString(gdPrevImage,gdFontSmall,gdPrevXmaxPx-strlen(gdPrevStrXmax)*gdPrevStrFontWidth,gdPrevHeight,(unsigned char*) gdPrevStrXmax,gdPrevColorLimits);
        gdImageLine(gdPrevImage,gdPrevXmaxPx-1,gdPrevHeight+gdPrevStrFontHeight,gdPrevXmaxPx-strlen(gdPrevStrXmax)*gdPrevStrFontWidth,gdPrevHeight+gdPrevStrFontHeight,gdPrevColorLimits);
                
        //X total
        char gdPrevStrXfull [14];
        sprintf (gdPrevStrXfull, " %.2lfmm ", limits[0].xMax - limits[0].xMin);
        if(numDiffDouble (limits[0].xMin, limits[0].xMax) > 0.001){
            NNSgdImageArrow(gdPrevImage,gdPrevXminPx,gdPrevHeight+gdPrevStrFontHeight*2,gdPrevStrFontHeight,6,6,3,gdPrevColorLimits,true);
            NNSgdImageArrow(gdPrevImage,gdPrevXmaxPx,gdPrevHeight+gdPrevStrFontHeight*2,gdPrevStrFontHeight,6,6,3,gdPrevColorLimits,true);
            gdImageLine(gdPrevImage,gdPrevXminPx+1,gdPrevHeight+gdPrevStrFontHeight*2,gdPrevXmaxPx-1,gdPrevHeight+gdPrevStrFontHeight*2,gdPrevColorLimits);
            gdImageString(gdPrevImage,gdFontSmall,(gdPrevXminPx+(gdPrevXmaxPx-gdPrevXminPx)/2)-strlen(gdPrevStrXfull)*gdPrevStrFontWidth/2,gdPrevHeight+gdPrevStrFontHeight,(unsigned char*) gdPrevStrXfull,gdPrevColorLimits);
        }

        //Y min
        char gdPrevStrYmin [14];
        sprintf (gdPrevStrYmin, " %.2lfmm ", limits[0].yMin);
        NNSgdImageArrow(gdPrevImage,gdPrevWidth+gdPrevStrFontHeight-1,gdPrevYminPx,gdPrevStrFontHeight,6,6,1,gdPrevColorLimits,true);
        gdImageStringUp(gdPrevImage,gdFontSmall,gdPrevWidth,gdPrevYminPx-2,(unsigned char*) gdPrevStrYmin,gdPrevColorLimits);
        gdImageLine(gdPrevImage,gdPrevWidth+gdPrevStrFontHeight,gdPrevYminPx-1,gdPrevWidth+gdPrevStrFontHeight,gdPrevYminPx-strlen(gdPrevStrYmin)*gdPrevStrFontWidth,gdPrevColorLimits);

        //Y max
        char gdPrevStrYmax [14];
        sprintf (gdPrevStrYmax, " %.2lfmm ", limits[0].yMax);
        NNSgdImageArrow(gdPrevImage,gdPrevWidth+gdPrevStrFontHeight-1,gdPrevYmaxPx,gdPrevStrFontHeight,6,6,1,gdPrevColorLimits,true);
        gdImageStringUp(gdPrevImage,gdFontSmall,gdPrevWidth,gdPrevYmaxPx+strlen(gdPrevStrYmax)*gdPrevStrFontWidth,(unsigned char*) gdPrevStrYmax,gdPrevColorLimits);
        gdImageLine(gdPrevImage,gdPrevWidth+gdPrevStrFontHeight,gdPrevYmaxPx+1,gdPrevWidth+gdPrevStrFontHeight,gdPrevYmaxPx+strlen(gdPrevStrYmax)*gdPrevStrFontWidth,gdPrevColorLimits);

        //Y total
        char gdPrevStrYfull [14];
        sprintf (gdPrevStrYfull, " %.2lfmm ", limits[0].yMax - limits[0].yMin);
        if(numDiffDouble (limits[0].yMin, limits[0].yMax) > 0.001){
            NNSgdImageArrow(gdPrevImage,gdPrevWidth+gdPrevStrFontHeight*2,gdPrevYminPx,gdPrevStrFontHeight,6,6,1,gdPrevColorLimits,true);
            NNSgdImageArrow(gdPrevImage,gdPrevWidth+gdPrevStrFontHeight*2,gdPrevYmaxPx,gdPrevStrFontHeight,6,6,1,gdPrevColorLimits,true);
            gdImageLine(gdPrevImage,gdPrevWidth+gdPrevStrFontHeight*2,gdPrevYminPx-1,gdPrevWidth+gdPrevStrFontHeight*2,gdPrevYmaxPx+1,gdPrevColorLimits);
            gdImageStringUp(gdPrevImage,gdFontSmall,gdPrevWidth+gdPrevStrFontHeight,(gdPrevYminPx+(gdPrevYmaxPx-gdPrevYminPx)/2)+strlen(gdPrevStrYfull)*gdPrevStrFontWidth/2,(unsigned char*) gdPrevStrYfull,gdPrevColorLimits);
        }

        //Z
        char gdPrevStrZ [100];
        sprintf (gdPrevStrZ, "Z min : %.2lfmm <> Z max : %.2lfmm", limits[0].zMin, limits[0].zMax);
        gdPrevStrWidth = strlen(gdPrevStrZ) * gdPrevStrFontWidth; gdPrevStrX = (gdPrevWidthFull - gdPrevStrWidth) / 2;
        gdImageString(gdPrevImage,gdFontSmall,gdPrevStrX ,gdPrevHeight+gdPrevStrFontHeight*(gdPrevTxtLines-3),(unsigned char*) gdPrevStrZ,gdPrevColorLimits);
        gdImageLine(gdPrevImage,gdPrevStrX,gdPrevHeight+gdPrevStrFontHeight*(gdPrevTxtLines-2),gdPrevStrX + gdPrevStrWidth ,gdPrevHeight+gdPrevStrFontHeight*(gdPrevTxtLines-2),gdPrevColorLimits);

        //output file
        sprintf (previewFilePath, "%s.png", file);
        gdFileHandle = fopen (previewFilePath, "wb");
        if (gdFileHandle != NULL) {
            //gdImagePngEx(gdPrevImage, gdFileHandle, 9); //around 10 time slower than gdImagePng
            gdImagePng(gdPrevImage, gdFileHandle);
            fclose(gdFileHandle);
            NNSPNGaddPPM (previewFilePath, (unsigned int)(gdPrevScale * 1000.)); //write proper ppm
        } else {gdImageDestroy(gdPrevImage); return 1;} //ERR1 : fail to write
        gdImageDestroy(gdPrevImage);
    } else {return 2;} //ERR2 : fail to create gdImagePtr
    return 0; //ok
}



