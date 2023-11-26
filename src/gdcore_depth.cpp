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

#include "gdcore_depth.h"


long bwPixelDrawn = 0; //used for debug


int NNSclampIntPermissive(int num, int min, int max, int limit){ //clamp int value withing range, if value over/under 'permissive', value is unchanged
    if (num < (min - limit) || num > (max + limit)){return num;}
    return NNSclampInt(num, min, max);
}


//image buffer related functions
NNSbwImagePtr NNSbwImageCreate(int width, int height){ //allocate image, to be freed with NNSbwImageDestroy(), transparentIndex: -1:none, 0:black, 1:white
    if (width < 1 || height < 1){
        //debug_stderr("invalid size (%d, %d)\n", width, height);
        return nullptr;
    }

    int bitmapArrSize = width * height;

    //allocate pixels buffer
    int* bitmap = new(std::nothrow)int[bitmapArrSize]();
    if (bitmap == nullptr){
        debug_stderr("failed to allocate pixels buffer, bitmapArrSize:%d\n", bitmapArrSize);
        return nullptr;
    }// else {debug_stderr("pixels buffer allocated\n");}
    for (int i = 0; i < bitmapArrSize; i++){*(bitmap + i) = -1;}

    //allocate y pointers array
    int** bitmapPtr = new(std::nothrow)int*[height]();
    if (bitmapPtr == nullptr){
        debug_stderr("failed to allocate pixel buffer Y pointers array (%d)\n", height);
        delete[] bitmap;
        return nullptr;
    }// else {debug_stderr("Y pointers array allocated\n");}
    for (int i = 0; i < height; i++){bitmapPtr[i] = bitmap + width * i;}

    //allocate image struct
    NNSbwImagePtr tmpImage = new(std::nothrow)NNSbwImage;
    if (tmpImage == nullptr){ //went wrong
        debug_stderr("failed to allocate image struct\n");
        delete[] bitmap;
        delete[] bitmapPtr;
        return nullptr;
    }// else {debug_stderr("image struct allocated\n");}

    tmpImage->width = width;
    tmpImage->height = height;
    tmpImage->bitmapPtr = bitmapPtr;
    tmpImage->bitmap = bitmap;

    return tmpImage;
}

void NNSbwImageDestroy(NNSbwImagePtr image){ //deallocate image, image pointer needs to be set to nullptr afterward
    if (image == nullptr){return;} 
    delete[] image->bitmapPtr; //free y buffers
    delete[] image->bitmap; //free pixels buffer
    delete image; //free image struct
}


//NNSbw to libGD bridge functions
void NNSbwImage2gdGreyscaleCopy(NNSbwImagePtr image, gdImagePtr gdImage){ //copy BW to gd true color image, color < 0 are not be copied, > 255 clamped to 255. Always copy from BW XY0 to GD XY0, overflow is ignored
    if (image == nullptr || gdImage == nullptr){return;}

    //precompute gd colors
    int *gdColors = new(std::nothrow)int[256];
    if (gdColors == nullptr){debug_stderr("failed to allocate gdColors buffer\n"); return;}
    for (int i = 0; i < 256; i++){gdColors[255 - i] = gdTrueColorAlpha(i, i, i, gdAlphaOpaque);}

    int gdWidth = gdImage->sx, gdHeight = gdImage->sy; //gd image
    int width = image->width, height = image->height; //bw image
    int maxWidth = (gdWidth > width) ? width : gdWidth, maxHeight = (gdHeight > height) ? height : gdHeight; //avoid offscreen drawing

    //copy to gd
    for (int y = 0; y < maxHeight; y++){
        int* bitmapPtr = image->bitmapPtr[y];
        for (int x = 0; x < maxWidth; x ++){
            int color = *(bitmapPtr + x);
            if (color < 0){continue;}
            if (color > 255){color = 255;}
            gdImage->tpixels[y][x] = gdColors[color];
        }
    }

    delete []gdColors;
}

//drawing functions
void NNSbwSetPixel(NNSbwImagePtr image, int x, int y, int color){ //set pixel color
    if (x < 0 || y < 0 || x >= image->width || y >= image->height){return;}
    if (color > image->bitmapPtr[y][x]){
        image->bitmapPtr[y][x] = color;
        bwPixelDrawn++;
    }
}

int NNSbwGetPixel(NNSbwImagePtr image, int x, int y){ //get pixel color
    if (x < 0 || y < 0 || x >= image->width || y >= image->height){return -1;}
    return image->bitmapPtr[y][x];
}

void NNSbwTriangle(NNSbwImagePtr image, int x1, int y1, int x2, int y2, int x3, int y3, int color){ //draw triangle, based on Adafruit GFX library fillTriangle()
    if (image == nullptr){return;}

    //sort coordinates by Y order (y3 >= y2 >= y1)
    int tmpInt;
    if (y1 > y2){tmpInt = y1; y1 = y2; y2 = tmpInt; tmpInt = x1; x1 = x2; x2 = tmpInt;}
    if (y2 > y3){tmpInt = y3; y3 = y2; y2 = tmpInt; tmpInt = x3; x3 = x2; x2 = tmpInt;}
    if (y1 > y2){tmpInt = y1; y1 = y2; y2 = tmpInt; tmpInt = x1; x1 = x2; x2 = tmpInt;}

    NNSbwLine(image, x1, y1, x2, y2, color);
    NNSbwLine(image, x2, y2, x3, y3, color);
    NNSbwLine(image, x3, y3, x1, y1, color);
}

void NNSbwTriangleFilled(NNSbwImagePtr image, int x1, int y1, int x2, int y2, int x3, int y3, int color){ //draw filled triangle, based on Adafruit GFX library fillTriangle()
    if (image == nullptr){return;}

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
        for (; a <= b; a++){NNSbwSetPixel(image, a, y1, color);}
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
        for (; a <= b; a++){NNSbwSetPixel(image, a, y, color);}
    }

    //for lower part of triangle, find scanline crossings for segments
    //0-2 and 1-2.  This loop is skipped if y2=y3.
    sa = dx22 * (y - y2); sb = dx12 * (y - y1);
    for (; y <= y3; y++){
        a = x2 + sa / dy22; b = x1 + sb / dy12;
        sa += dx22; sb += dx12;
        if (a > b){tmpInt = a; a = b; b = tmpInt;}
        for (; a <= b; a++){NNSbwSetPixel(image, a, y, color);}
    }
}

void NNSbwPolygonFilled(NNSbwImagePtr image, NNSbwPointPtr pointArr, int pointCount, int color){ //draw filled polygon
    if (image == nullptr){return;}
    if (pointCount <= 0){return;}

    if (pointCount == 1){NNSbwSetPixel(image, pointArr[0].x, pointArr[0].y, color); return;}
    if (pointCount == 2){NNSbwLine(image, pointArr[0].x, pointArr[0].y, pointArr[1].x, pointArr[1].y, color); return;}
    if (pointCount == 3){NNSbwTriangleFilled(image, pointArr[0].x, pointArr[0].y, pointArr[1].x, pointArr[1].y, pointArr[2].x, pointArr[2].y, color); return;}

    bool upTriangle = true;
    int upLast = 0, downLast = pointCount - 1;
    while ((downLast - upLast) > 0){
        if (upTriangle){
            NNSbwTriangleFilled(image, pointArr[upLast].x, pointArr[upLast].y, pointArr[upLast + 1].x, pointArr[upLast + 1].y, pointArr[downLast].x, pointArr[downLast].y, color);
            upLast++;
        } else {
            NNSbwTriangleFilled(image, pointArr[downLast].x, pointArr[downLast].y, pointArr[downLast - 1].x, pointArr[downLast - 1].y, pointArr[upLast].x, pointArr[upLast].y, color);
            downLast--;
        }
        upTriangle = !upTriangle;
    }
}

void NNSbwCircleFilled(NNSbwImagePtr image, int cx, int cy, int d, int color){ //draw filled circle, based on Adafruit GFX library fillCircleHelper()
    if (image == nullptr){return;}
    if (d <= 1){NNSbwSetPixel(image, cx, cy, color); return;}

    int r = d / 2, f = 1 - r;
    int ddF_x = 1, ddF_y = -2 * r;
    int x = 0, y = r;
    int px = x, py = y;
    while (x < y){
        if (f >= 0){y--; ddF_y += 2; f += ddF_y;}
        x++; ddF_x += 2; f += ddF_x;
        if (x < (y + 1)){
            for (int tmpY = cy - y; tmpY <= cy + y; tmpY++){
                NNSbwSetPixel(image, cx + x, tmpY, color);
                NNSbwSetPixel(image, cx - x, tmpY, color);
            }
        }
        if (y != py){
            for (int tmpY = cy - px; tmpY <= cy + px; tmpY++){
                NNSbwSetPixel(image, cx + py, tmpY, color);
                NNSbwSetPixel(image, cx - py, tmpY, color);
            }
            py = y;
        }
        px = x;
    }
    for (int tmpY = cy - r; tmpY <= cy + r; tmpY++){NNSbwSetPixel(image, cx, tmpY, color);}
}

void NNSbwRectangleFilled(NNSbwImagePtr image, int x1, int y1, int x2, int y2, int color){ //draw filled rectangle
    if (image == nullptr){return;}

    if (x1 > x2){int tmpInt = x1; x1 = x2; x2 = tmpInt;}
    if (y1 > y2){int tmpInt = y1; y1 = y2; y2 = tmpInt;}

    for (; y1 <= y2; y1++){
        for (int x = x1; x <= x2; x++){NNSbwSetPixel(image, x, y1, color);}
    }
}

void NNSbwLineHorizontal(NNSbwImagePtr image, int x, int y, int width, int color){ //draw horizontal line
    if (image == nullptr){return;}
    if (width < 0){width = abs(width); x -= width;}
    for (int xCurr = x; xCurr <= x + width; xCurr++){NNSbwSetPixel(image, xCurr, y, color);}
}

void NNSbwLineVertical(NNSbwImagePtr image, int x, int y, int height, int color){ //draw vertical line
    if (image == nullptr){return;}
    if (height < 0){height = abs(height); y -= height;}
    for (int yCurr = y; yCurr <= y + height; yCurr++){NNSbwSetPixel(image, x, yCurr, color);}
}

void NNSbwLine(NNSbwImagePtr image, int x1, int y1, int x2, int y2, int color){ //draw line, based on Adafruit GFX library writeLine()
    if (image == nullptr){return;}

    if (x1 == x2){ //vertical line
        if (y1 == y2){NNSbwSetPixel(image, x1, y1, color); return;} //single point
        if (y1 > y2){int tmp = y1; y1 = y2; y2 = tmp;}
        for (; y1 <= y2; y1++){NNSbwSetPixel(image, x1, y1, color);}
        return;
    }
    
    if (y1 == y2){ //horizontal line
        if (x1 == x2){NNSbwSetPixel(image, x1, y1, color); return;} //single point
        if (x1 > x2){int tmp = x1; x1 = x2; x2 = tmp;}
        for (; x1 <= x2; x1++){NNSbwSetPixel(image, x1, y1, color);}
        return;
    }

    //based on Adafruit GFX library writeLine() as it fairly accurate and fast
    bool steep = abs(y2 - y1) > abs(x2 - x1);
    if (steep){ //swap x and y
        int tmpInt = x1; x1 = y1; y1 = tmpInt;
        tmpInt = x2; x2 = y2; y2 = tmpInt;
    }

    if (x1 > x2){ //ensure x1 always lower than x2
        int tmpInt = x1; x1 = x2; x2 = tmpInt;
        tmpInt = y1; y1 = y2; y2 = tmpInt;
    }

    int diffX = x2 - x1, diffY = abs(y2 - y1);
    int err = diffX / 2;

    int stepY = -1;
    if (y1 < y2){stepY = 1;}

    for (; x1 <= x2; x1++){
        if (steep){NNSbwSetPixel(image, y1, x1, color);} else {NNSbwSetPixel(image, x1, y1, color);}
        err -= diffY;
        if (err < 0){y1 += stepY; err += diffX;}
    }
}

void NNSbwLineThick(NNSbwImagePtr image, float x1, float y1, float x2, float y2, float thickness, int color){ //draw thick line
    if (image == nullptr){return;}
    if (thickness <= 1.f){NNSbwLine(image, x1, y1, x2, y2, color); return;}
    NNSbwCircleFilled(image, x1, y1, thickness, color);
    NNSbwCircleFilled(image, x2, y2, thickness, color);
    float halfthick = thickness / 2;

    if (x1 == x2){
        NNSbwRectangleFilled(image, x1 - halfthick, y1, x2 + halfthick, y2, color);
        return;
    } else if (y1 == y2){
        NNSbwRectangleFilled(image, x1, y1 - halfthick, x2, y2 + halfthick, color);
        return;
    } else {
        float angle = atan((y2 - y1) / (x2 - x1));
        float deccos = halfthick * cos(angle);
        float decsin = halfthick * sin(angle);
        float px0 = x1 + decsin, py0 = y1 - deccos;
        float px1 = x1 - decsin, py1 = y1 + deccos;
        float px2 = x2 - decsin, py2 = y2 + deccos;
        float px3 = x2 + decsin, py3 = y2 - deccos;
        NNSbwTriangleFilled(image, px0, py0, px1, py1, px3, py3, color);
        NNSbwTriangleFilled(image, px3, py3, px1, py1, px2, py2, color);
    }
}

void NNSbwLineThickDepth(NNSbwImagePtr image, int bitmapWidth, int bitmapHeight, int startLayer, int endLayer, float x1, float y1, float x2, float y2, float thickness){ //draw thick line that can cross multiple depth buffer image
    if (startLayer == endLayer/* && startLayer >= 0 && startLayer < 256*/){
        NNSbwLineThick(image, x1, y1, x2, y2, thickness, startLayer);
        return;
    }

    float lineLength = abs(sqrt((float)(y2 - y1) * (float)(y2 - y1) + (float)(x2 - x1) * (float)(x2 - x1)));
    if (lineLength == 0.f){lineLength = 0.001;} //deal with start = end
    int layerDiff = abs(endLayer - startLayer); if (layerDiff == 0){layerDiff++;}
    float stepLength = lineLength / layerDiff;
    float halfthick = thickness / 2.f;
    float angle = atan((float)(y2 - y1) / (x2 - x1)), deccos = halfthick * cos(angle), decsin = halfthick * sin(angle);
    float xShift = (float)(x2 - x1) / layerDiff, yShift = (float)(y2 - y1) / layerDiff;
    float currentX = x1, currentY = y1, lastX = x1, lastY = y1;
    float px0, py0, px1, py1, px2, py2, px3, py3;

    for (float currentLength = 0; currentLength < lineLength; currentLength += stepLength){
        int layer = NNSintFade(startLayer, endLayer, 1. - (lineLength - currentLength) / lineLength);
        currentX += xShift; currentY += yShift;

        if (layer >= 0/* && layer < 256*/){
            if (halfthick < .6f){
                NNSbwLine(image, lastX, lastY, currentX, currentY, layer);
                continue;
            }

            px0 = lastX + decsin; py0 = lastY - deccos;
            px3 = currentX + decsin; py3 = currentY - deccos;

            px1 = lastX - decsin; py1 = lastY + deccos;
            px2 = currentX - decsin; py2 = currentY + deccos;
            NNSbwTriangleFilled(image, px0, py0, px1, py1, px3, py3, layer);
            NNSbwTriangleFilled(image, px3, py3, px1, py1, px2, py2, layer);

            NNSbwCircleFilled(image, lastX, lastY, thickness, layer);
            NNSbwCircleFilled(image, currentX, currentY, thickness, layer);
        }
        lastX = currentX; lastY = currentY;
    }
}

void NNSbwLineThickVDepth(NNSbwImagePtr image, int bitmapWidth, int bitmapHeight, int startLayer, int endLayer, float x1, float y1, float x2, float y2, float toolDia, float toolAngle, float layerSpacing){ //draw thick line with different start and end thickness that can cross multiple depth buffer image
    float lineLength = abs(sqrt((float)(y2 - y1) * (float)(y2 - y1) + (float)(x2 - x1) * (float)(x2 - x1)));
    if (lineLength == 0.f){lineLength = 0.001;} //deal with start = end
    int layerDiff = abs(endLayer - startLayer); if (layerDiff == 0){layerDiff++;}
    float stepLength = lineLength / layerDiff;
    float angle = atan((float)(y2 - y1) / (x2 - x1));
    float xShift = (float)(x2 - x1) / layerDiff, yShift = (float)(y2 - y1) / layerDiff;
    float currentX = x1, currentY = y1, lastX = x1, lastY = y1;
    float halfthickShift = tan(deg2rad(toolAngle)) * layerSpacing, halfDmax = toolDia / 2.f;
    float tmpCos = cos(angle), tmpSin = sin(angle), px0, py0, px1, py1, px2, py2, px3, py3;

    for (float currentLength = 0; currentLength < lineLength; currentLength += stepLength){
        int layer = NNSintFade(startLayer, endLayer, 1. - (lineLength - currentLength) / lineLength);
        currentX += xShift; currentY += yShift;
        if (layer >= 0/* && layer < 256*/){
            float halfthick = 0.f;
            for (; layer >= 0; layer--){
                if (halfthick > halfDmax){continue;}
                if (halfthick < .6f){
                    NNSbwLine(image, lastX, lastY, currentX, currentY, layer);
                    halfthick += halfthickShift;
                    continue;
                }

                float deccos = halfthick * tmpCos, decsin = halfthick * tmpSin;
                px0 = lastX + decsin; py0 = lastY - deccos;
                px3 = currentX + decsin; py3 = currentY - deccos;
                NNSbwCircleFilled(image, lastX, lastY, halfthick * 2, layer);

                halfthick += halfthickShift;
                deccos = halfthick * tmpCos, decsin = halfthick * tmpSin;
                px1 = lastX - decsin; py1 = lastY + deccos;
                px2 = currentX - decsin; py2 = currentY + deccos;
                NNSbwCircleFilled(image, currentX, currentY, halfthick * 2, layer);

                NNSbwTriangleFilled(image, px0, py0, px1, py1, px3, py3, layer);
                NNSbwTriangleFilled(image, px3, py3, px1, py1, px2, py2, layer);
            }
        }
        lastX = currentX; lastY = currentY;
    }
}

void NNSbwArc(NNSbwImagePtr image, float cx, float cy, float width, float height, float start, float end, int color, int resolution){ //draw arc
    if (image == nullptr){return;}
    if (resolution < 1){resolution = 20;}
    float a1 = deg2rad(start), a2 = deg2rad(end), arclenght = abs(a2 - a1) * (height / 2), hwidth = width/2, hheight = height/2;
    int stepCount = (float)ceil(arclenght / resolution) + 1;
    float stepangle = (a2 - a1) / stepCount;
    float lastx = cx + cos(a1) * hwidth, lasty = cy + sin(a1) * hheight, x, y;
    for (int i = 0; i < stepCount + 1; i++){
        x = cx + cos(a1 + stepangle * i) * hwidth;
        y = cy + sin(a1 + stepangle * i) * hheight;
        NNSbwLine(image, x, y, lastx, lasty, color);
        lastx = x; lasty = y;
    }
}

void NNSbwArcThick(NNSbwImagePtr image, float cx, float cy, float width, float height, float start, float end, float thickness, int color, int resolution){ //draw thick arc
    if (image == nullptr){return;}
    if (resolution < 1){resolution = 20;}
    if (thickness <= 1.f){NNSbwArc(image, cx, cy, width, height, start, end, color, resolution); return;}

    float a1 = deg2rad(start), a2 = deg2rad(end), arclenght = abs(a2 - a1) * (height / 2), hwidth = width/2, hheight = height/2;
    int stepCount = (float)ceil(arclenght / resolution) + 1;
    float stepangle = (a2 - a1) / stepCount;
    float radiusxmin = hwidth - thickness / 2, radiusymin = hheight - thickness / 2, radiusxmax = hwidth + thickness / 2, radiusymax = hheight + thickness / 2;

    NNSbwCircleFilled(image, cx + cos(a1) * hwidth, cy + sin(a1) * hheight, thickness, color);
    NNSbwCircleFilled(image, cx + cos(a2) * hwidth, cy + sin(a2) * hheight, thickness, color);

    bool first = true;
    float tmpa, tmpCos, tmpSin, px0, py0, px1, py1, px2, py2, px3, py3;
    for (int i = 0; i < stepCount + 1; i++){
        tmpa = a1 + (stepangle * i);
        tmpCos = cos(tmpa); tmpSin = sin(tmpa);
        if (first){
            px0 = cx + tmpCos * radiusxmin; py0 = cy + tmpSin * radiusymin;
            px1 = cx + tmpCos * radiusxmax; py1 = cy + tmpSin * radiusymax;
            first = false;
            continue;
        }
        px2 = cx + tmpCos * radiusxmin; py2 = cy + tmpSin * radiusymin;
        px3 = cx + tmpCos * radiusxmax; py3 = cy + tmpSin * radiusymax;
        NNSbwTriangleFilled(image, px0, py0, px2, py2, px3, py3, color);
        NNSbwTriangleFilled(image, px0, py0, px1, py1, px3, py3, color);
        px0 = px2; py0 = py2;
        px1 = px3; py1 = py3;
    }
}

void NNSbwArcThickDepth(NNSbwImagePtr image, int bitmapWidth, int bitmapHeight, int startLayer, int endLayer, float cx, float cy, float width, float height, float start, float end, float thickness, int resolution){ //draw thick arc that can cross multiple depth buffer image
    if (resolution < 1){resolution = 20;}
    if (startLayer == endLayer/* && startLayer >= 0 && startLayer < 256*/){
        NNSbwArcThick(image, cx, cy, width, height, start, end, thickness, startLayer, resolution);
        return;
    }
    
    float angle1 = deg2rad(start), angle2 = deg2rad(end), angleDiff = angle2 - angle1;
    int stepCount = ceil(abs(angleDiff) * ((float)height / 2.f) / (float)resolution) + 1; if (stepCount == 0){stepCount++;}
    float stepAngle = abs(angleDiff) / (float)stepCount, hwidth = width / 2, hheight = height / 2;
    float halfthick = thickness / 2;
    float radiusxmin = hwidth - halfthick, radiusymin = hheight - halfthick;
    float radiusxmax = hwidth + halfthick, radiusymax = hheight + halfthick;

    if (angle1 > angle2){
        float tmpFloat = angle1; angle1 = angle2; angle2 = tmpFloat;
        int tmpInt = startLayer; startLayer = endLayer; endLayer = tmpInt;
    }
    
    bool first = true;
    float tmpCos, tmpSin, tmpCos1, tmpSin1, px0, py0, px1, py1, px2, py2, px3, py3;

    for (float currentAngle = angle1; currentAngle < angle2; currentAngle += stepAngle){
        int layer = NNSintFade(startLayer, endLayer, 1. - (angle2 - currentAngle) / abs(angleDiff));

        if (layer >= 0/* && layer < 256*/){
            if (first){tmpCos = cos(currentAngle); tmpSin = sin(currentAngle);}
            tmpCos1 = cos(currentAngle + stepAngle); tmpSin1 = sin(currentAngle + stepAngle);

            if (halfthick < .6f){
                px0 = cx + tmpCos * hwidth; py0 = cy + tmpSin * hheight;
                px1 = cx + tmpCos1 * hwidth; py1 = cy + tmpSin1 * hheight;
                NNSbwLine(image, px0, py0, px1, py1, layer);
                continue;
            }

            if (first){
                px0 = cx + tmpCos * radiusxmin; py0 = cy + tmpSin * radiusymin;
                px3 = cx + tmpCos * radiusxmax; py3 = cy + tmpSin * radiusymax;
                first = false;
            }

            px1 = cx + tmpCos1 * radiusxmin; py1 = cy + tmpSin1 * radiusymin;
            px2 = cx + tmpCos1 * radiusxmax; py2 = cy + tmpSin1 * radiusymax;
            
            NNSbwCircleFilled(image, cx + tmpCos * hwidth, cy + tmpSin * hheight, thickness, layer);
            NNSbwCircleFilled(image, cx + tmpCos1 * hwidth, cy + tmpSin1 * hheight, thickness, layer);
            NNSbwTriangleFilled(image, px0, py0, px1, py1, px3, py3, layer);
            NNSbwTriangleFilled(image, px3, py3, px1, py1, px2, py2, layer);
        }
        
        px0 = px1; py0 = py1;
        px3 = px2; py3 = py2;
        tmpCos = tmpCos1; tmpSin = tmpSin1;
    }
}

void NNSbwArcThickVDepth(NNSbwImagePtr image, int bitmapWidth, int bitmapHeight, int startLayer, int endLayer, float cx, float cy, float width, float height, float startAngle, float endAngle, float toolDia, float toolAngle, float layerSpacing, int resolution){ //draw thick arc with different start and end thickness that can cross multiple depth buffer image
    if (resolution < 1){resolution = 20;}

    float angle1 = deg2rad(startAngle), angle2 = deg2rad(endAngle), angleDiff = angle2 - angle1;
    int stepCount = ceil(abs(angleDiff) * ((float)height / 2.f) / (float)resolution) + 1; if (stepCount <= 0){stepCount = 1;}
    float stepAngle = abs(angleDiff) / (float)stepCount, hwidth = width / 2, hheight = height / 2;
    float halfthickShift = tan(deg2rad(toolAngle)) * layerSpacing, halfDmax = toolDia / 2.f;
    float tmpCos, tmpSin, tmpCos1, tmpSin1, px0, py0, px1, py1, px2, py2, px3, py3;
    bool first = true;

    if (angle1 > angle2){
        float tmpFloat = angle1; angle1 = angle2; angle2 = tmpFloat;
        int tmpInt = startLayer; startLayer = endLayer; endLayer = tmpInt;
    }

    for (float currentAngle = angle1; currentAngle < angle2; currentAngle += stepAngle){
        int layer = NNSintFade(startLayer, endLayer, 1. - (angle2 - currentAngle) / abs(angleDiff));
        if (layer >= 0/* && layer < 256*/){
            float halfthick = 0.f;
            if (first){tmpCos = cos(currentAngle); tmpSin = sin(currentAngle); first = false;}
            tmpCos1 = cos(currentAngle + stepAngle); tmpSin1 = sin(currentAngle + stepAngle);
            for (; layer >= 0; layer--){
                if (halfthick > halfDmax){continue;}
                if (halfthick < .6f){
                    px0 = cx + tmpCos * hwidth; py0 = cy + tmpSin * hheight;
                    px1 = cx + tmpCos1 * hwidth; py1 = cy + tmpSin1 * hheight;
                    NNSbwLine(image, px0, py0, px1, py1, layer);
                    halfthick += halfthickShift;
                    continue;
                }

                px0 = cx + tmpCos * (hwidth - halfthick); py0 = cy + tmpSin * (hheight - halfthick);
                px3 = cx + tmpCos * (hwidth + halfthick); py3 = cy + tmpSin * (hheight + halfthick);
                NNSbwCircleFilled(image, cx + tmpCos * (hwidth), cy + tmpSin * (hheight), halfthick * 2, layer);

                halfthick += halfthickShift;
                px1 = cx + tmpCos1 * (hwidth - halfthick); py1 = cy + tmpSin1 * (hheight - halfthick);
                px2 = cx + tmpCos1 * (hwidth + halfthick); py2 = cy + tmpSin1 * (hheight + halfthick);
                NNSbwCircleFilled(image, cx + tmpCos1 * (hwidth), cy + tmpSin1 * (hheight), halfthick * 2, layer);

                NNSbwTriangleFilled(image, px0, py0, px1, py1, px3, py3, layer);
                NNSbwTriangleFilled(image, px3, py3, px1, py1, px2, py2, layer);
            }
            tmpCos = tmpCos1;
            tmpSin = tmpSin1;
        }
    }
}

