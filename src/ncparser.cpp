/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to gcode parsing.

Note: NCparseFile() will always convert coordonate system to absolute

upd 0.4a ok
*/

#include "ncparser_language.h"
#include "ncparser.h"

bool inArrayInt(int* arr, int value, int arrSize){ //check if int in array
    for (int i=0; i < arrSize; i++) {if (arr[i] == value){return true;}}
    return false;
}

char *strtok_chr(char* str, const char delimiter){ //custom implement of strtok with char delimiter, around 20% faster
    static char *nextPtr = NULL;
    char* currentPtr;

    if (str == NULL){
        if (nextPtr == NULL){return NULL;}
        currentPtr = nextPtr;
    } else {currentPtr = nextPtr = str;}

    while (*nextPtr != delimiter && *nextPtr != '\0'){nextPtr++;} //strchr replacement
    if (*nextPtr == '\0'){nextPtr = NULL;} else {*nextPtr++ = '\0';} //"tokenize"
    return currentPtr;
}

void secToClock(char* arr, double value){ //convert seconds to char array in format : MM:SS / H:MM:SS
    int hours = 0, minutes = 0, seconds = 0;
    hours = floor(value / 3600);
    minutes = floor((value - (hours * 3600)) / 60);
    seconds = value - ((hours * 3600) + (minutes * 60));
    if (hours > 0){
        sprintf(arr, "%d:%02d:%02d", hours, minutes, seconds);
        return;
    }
    sprintf(arr, "%02d:%02d", minutes, seconds);
}

void charArrFill(char* arr, unsigned int length, char chr){ //fill char array with given char, array will be null terminated 
    for (unsigned int i = 0; i < length - 2; i++){*arr++ = chr;} *arr = '\0';
}

double numDiffDouble(double n1, double n2){return abs((n1 > n2) ? n2 - n1 : n1 - n2);} //absolute delta between 2 doubles
float numDiffFloat(float n1, float n2){return abs((n1 > n2) ? n2 - n1 : n1 - n2);} //absolute delta between 2 floats

double rad2deg(double rad){return rad * 57.29577951308232286;} //convert radian to degree

double deg2rad(double deg){return deg * 0.01745329251994329547;} //convert degree to radian

double angle3points(double centerX, double centerY, double x1, double y1, double x2, double y2){ //compute angle based on start/end/center coordonates of a arc
    double v1x = x2 - centerX; double v1y = y2 - centerY;
    double v2x = x1 - centerX; double v2y = y1 - centerY;
    double vcos = (v1x * v2x + v1y * v2y) / (sqrt (v1x*v1x + v1y*v1y) * sqrt (v2x*v2x + v2y*v2y));
    if (vcos < -0.9999){vcos = -0.9999;} else if (vcos > 0.9999){vcos = 0.9999;}
    return acos(vcos);
}

void arcLimits(double* limitArr, double centerX, double centerY, double width, double height, double angleStart, double angleEnd, int resolution){ //compute arc limits
    if (resolution < 1){resolution = 20;}
    double hwidth = width / 2; double hheight = height / 2;
    double angle1 = deg2rad(angleStart);
    double x1 = centerX + cos(angle1) * hwidth;
    double y1 = centerY - sin(angle1) * hheight;
    double angle2 = deg2rad(angleEnd);
    double x2 = centerX + cos(angle2) * hwidth;
    double y2 = centerY - sin(angle2) * hheight;
    double xMin = x1, xMax = x2, yMin = y1, yMax = y2, x, y;

    if (x1 > x2){xMin = x2; xMax = x1;}
    if (y1 > y2){yMin = y2; yMax = y1;}

    //if (x1 > x2){xMin = x2; xMax = x1;} else {xMin = x1; xMax = x2;}
    //if (y1 > y2){yMin = y2; yMax = y1;} else {yMin = y1; yMax = y2;}

    double arcLen = (angle2 - angle1) * (height / 2);
    int step = ceil(arcLen / resolution);
    double stepangle = (angle2 - angle1) / step;

    for (int i = 1; i < (step + 1); i++){
        x = centerX + cos(angle1 + stepangle * i) * hwidth;
        y = centerY - sin(angle1 + stepangle * i) * hheight;
        if (x < xMin){xMin = x;}
        if (x > xMax){xMax = x;}
        if (y < yMin){yMin = y;}
        if (y > yMax){yMax = y;}
    }
    limitArr[0] = xMin;
    limitArr[1] = xMax;
    limitArr[2] = yMin;
    limitArr[3] = yMax;
}

unsigned int ncCountLines(char* file){ //count number of lines into a file, ignore empty or commented lines (except if starts by a space)
    FILE* tmpHandle = fopen(file, "r");
    if (tmpHandle != NULL){
        char tmpBuffer[4096];
        unsigned int lineCount = 1;
        while(fgets(tmpBuffer, 4095, tmpHandle)){
            char tmpChar = *tmpBuffer;
            if (tmpChar != '(' && tmpChar != ';' && tmpChar != '\r' && tmpChar != '\n' && tmpChar != '\0'){lineCount++;}
        }
        fclose(tmpHandle);
        return lineCount + 1;
    }
    return 0;
}

int ncParseFile(char* file, unsigned int maxSpeedXY, unsigned int maxSpeedZ, ncFlagsStruct* flags, ncLineStruct* lines, ncToolStruct* tools, ncCommentStruct* comments, ncLimitStruct* limits, ncSummaryStruct* summary, ncArraySize* arrSizes, bool debugOutput){ //extract data from gcode file
    bool debugBack = debug;
    debug = debugOutput;

    //TODO: implement maxSpeedZ
    FILE *ncFileHandle = fopen(file, "r"); //gcode file stream
    if (ncFileHandle == NULL){return -ENOENT;}

    FILE *ncFileDebugHandle; //debug file handle
    char *ncFileDebugBuffer = nullptr;
    if (debugOutput){
        char strFileDebug[PATH_MAX]; sprintf(strFileDebug, "%s%s", file, "_parserDebug.txt");
        ncFileDebugHandle = fopen(strFileDebug, "wb");
        if (ncFileDebugHandle == NULL){
            debugOutput = false;
        } else {
            ncFileDebugBuffer = new(std::nothrow)char[streamBufferSize + 1];
            if (ncFileDebugBuffer != nullptr){
                debug_stderr("ncFileDebugBuffer allocated\n");
                setvbuf(ncFileDebugHandle, ncFileDebugBuffer, _IOFBF, streamBufferSize);
            } else {debug_stderr("Warning: failed to allocate ncFileDebugBuffer\n");}
        }
    }

    unsigned int currentLine = 1; //current line in file
    unsigned int toolNum = 0; //detected tool number
    unsigned int lineIndex = 1, commentIndex = 0; //arrays current indexes, lineIndex=1 to avoid troubles with line render

    int GcompatibleArr[] = {0,1,2,3,80,81,82,83,98}; //current compatible G functions
    int Gcurrent = 0, Gback = 0; //current G function
    int GflagUnit = -1; //G20:inch (TO BE IMPLEMENTED), G21:mm
    int GflagPlane = -1; //G17:XY, G18:ZX (TO BE IMPLEMENTED), G19:YZ (TO BE IMPLEMENTED)
    int GflagComp = -1; //G40:none, G41:left , G42:right, Note: G41 and G42 will never be implemented because of the huge amount of work required
    int GflagCoord = -1, GflagCircular = -1; //linear: G90:absolute, G91:relative, circular: G90.1:absolute, G91.1:relative
    int GflagRemaining = 5; //remaining G flags to be defined

    int Fcurrent = maxSpeedXY, Ffast = maxSpeedXY; //current and fast speed
    double Xcurrent = 0. , Xback = 0., Ycurrent = 0. , Yback = 0., Zcurrent = 0. , Zback = 0., Icurrent = 0., Jcurrent = 0., Kcurrent = 0., Qcurrent = 0., Rcurrent = 0.; //current and last position
    bool Xfirst = true, Yfirst = true, Zfirst = true; //first coordonate to be detected
    bool Iset = false, Jset = false, Kset = false;
    bool cvBlockDetected = false, cvBlockDetectedValid = false; //cutviewer block definition set
    int drillRetractMode = 0; double drillZinit = 0; //drill specific

    char strBuffer[4096]; //line buffer
    while (fgets(strBuffer, 4095, ncFileHandle)){ //read gcode file
        //clean current line
        char *strBufferPtr = strBuffer;
        char strCleanedBuffer[4096]; char *strCleanedBufferPtr = strCleanedBuffer;
        if (currentLine == 1 && strstr(strBufferPtr, "\xEF\xBB\xBF") != NULL){strBufferPtr += 3;} //remove utf8 bom
        while (*strBufferPtr != '\r' && *strBufferPtr != '\n' && *strBufferPtr != '\0'){ //not eol
            if (*strBufferPtr == '\t'){*strBufferPtr = ' ';} //replace tabulations by spaces
            *strCleanedBufferPtr++ = tolower(*strBufferPtr);
            if (*strBufferPtr == ' '){
                if (strBufferPtr == strBuffer){strCleanedBufferPtr--;} //remove line leading spaces
                while (*strBufferPtr == ' ' || *strBufferPtr == '\t'){strBufferPtr++;} //allow only one consecutive space
                continue;
            }
            strBufferPtr++;
        }
        *strCleanedBufferPtr = '\0';
        strCleanedBufferPtr = strCleanedBuffer; //reset poiter position for empty lines with comments

        if(debugOutput){
            *strBufferPtr = '\0';
            fprintf(ncFileDebugHandle, "%s\n",strBuffer);
            fprintf(ncFileDebugHandle, "CLEANED: %s\n",strCleanedBuffer);
        }
        
        //empty line, direct jump to next line
        if (*strCleanedBufferPtr == '\0'){
            summary->skip++;
            currentLine++;
            continue;
        }

        //parse line
        bool Gvalid = false, emptyLine = false;
        strcpy(strBuffer, strCleanedBuffer);
        //printf("%d:strBuffer(%d): %s\n", currentLine, strlen(strBuffer), strBuffer);
        char *tokenPtr = strtok_chr(strCleanedBuffer, ' '); //split line
        while (tokenPtr != NULL){ //loop thru tokens
            //printf("%d:tokenPtr(%d): %s\n", currentLine, strlen(tokenPtr), tokenPtr);

            char currentChar = *tokenPtr++;
            int tmpInt = atoi(tokenPtr); double tmpDouble = atof(tokenPtr);
            if (currentChar == '(' || currentChar == ';'){goto commentParseJump;} //avoid glitch caused by next line
            if (*tokenPtr == '\0'){break;} //nothing to do
            
            if (currentChar == 'g'){ //function
                Gcurrent = tmpInt; //new function
                if (inArrayInt(GcompatibleArr, Gcurrent, 9)){Gvalid = true;} else {Gcurrent = Gback;}

                if (GflagRemaining != 0){ //some flags are not declared
                    if (GflagUnit == -1 && (Gcurrent == 20 || Gcurrent == 21)){ //G20:inch, G21:mm
                        flags->unit = GflagUnit = Gcurrent; Gcurrent = Gback;
                        if(debugOutput){fprintf (ncFileDebugHandle, "PARSER: Flag:unit:%i\n", GflagUnit);}
                        goto GflagRemainingEnd;
                    }
                    
                    if (GflagPlane == -1 && (Gcurrent == 17 || Gcurrent == 18 || Gcurrent == 19)){ //G17:XY, G18:ZX, G19:YZ
                        flags->workplane = GflagPlane = Gcurrent; Gcurrent = Gback;
                        if(debugOutput){fprintf (ncFileDebugHandle, "PARSER: Flag:plane:%i\n", GflagPlane);}
                        goto GflagRemainingEnd;
                    }
                    
                    if (GflagComp == -1 && (Gcurrent == 40 || Gcurrent == 41 || Gcurrent == 42)){ //G40:none, G41:left, G42:right
                        flags->compensation = GflagComp = Gcurrent; Gcurrent = Gback;
                        if(debugOutput){fprintf (ncFileDebugHandle, "PARSER: Flag:comp:%i\n", GflagComp);}
                        goto GflagRemainingEnd;
                    }
                    
                    if (GflagCircular == -1 && strchr(tokenPtr, '.') != NULL && (Gcurrent == 90 || Gcurrent == 91)){ //circular: G90.1:absolute, G91.1:relative
                        flags->circular = GflagCircular = Gcurrent; Gcurrent = Gback;
                        if(debugOutput){fprintf (ncFileDebugHandle, "PARSER: Flag:circular:%i\n", GflagCircular);}
                        goto GflagRemainingEnd;
                    }
                    
                    if (GflagCoord == -1 && (Gcurrent == 90 || Gcurrent == 91)){ //linear: G90:absolute, G91:relative
                        flags->coord = GflagCoord = Gcurrent; Gcurrent = Gback;
                        if(debugOutput){fprintf (ncFileDebugHandle, "PARSER: Flag:linear:%i\n", GflagCoord);}
                    }

                    GflagRemainingEnd:;
                    GflagRemaining = (GflagUnit != -1) + (GflagPlane != -1) + (GflagComp != -1) + (GflagCoord != -1) + (GflagCircular != -1);
                }
            } else if (currentChar == '(' || currentChar == ';'){ //comment
                commentParseJump:;
                summary->commented++;

                //nothing else than a comment in this line
                if (strCleanedBufferPtr + 1 == tokenPtr){
                    summary->skip++;
                    emptyLine = true;
                }

                //comment: cutview tool definition: {$comment} TOOL/MILL,{$tool.diameter},{$tool.radius},{$tool.length},{$tool.angle} {$endcomment}
                char* tmpPtr = strstr(strBuffer, "tool/mill");
                if (tmpPtr != NULL){
                    float* tmpArray[5] = {NULL, &tools[toolNum].diameter, &tools[toolNum].radius, &tools[toolNum].length, &tools[toolNum].angle};
                    tokenPtr = strtok_chr(tmpPtr, ',');
                    for (unsigned int i = 0; i < 5; i++){
                        if (tokenPtr != NULL){
                            if (tmpArray[i] != NULL && !sscanf(tokenPtr, "%f", tmpArray[i])){
                                tmpInt = 0; sscanf(tokenPtr, "%d", &tmpInt);
                                *tmpArray[i] = (float)tmpInt;
                            }
                            tokenPtr = strtok_chr(NULL, ','); //next token
                        }
                    }
                    if (debugOutput){fprintf(ncFileDebugHandle, "PARSER: CV: Tool detected : T:%d, dia:%f, rad:%f, len:%f, ang:%f\n", toolNum, tools[toolNum].diameter, tools[toolNum].radius, tools[toolNum].length, tools[toolNum].angle);}
                    break;
                }

                //comment: cutview block definition: {$comment} STOCK/BLOCK,{$stock_width},{$stock_length},{$stock_height},{$stock_x},{$stock_y},{$stock_z} {$endcomment}
                if (!cvBlockDetected){
                    tmpPtr = strstr(strBuffer, "stock/block");
                    if (tmpPtr != NULL){
                        float* tmpArray[7] = {NULL, &(limits->xMax), &(limits->yMax), &(limits->zMax), &(limits->xMin), &(limits->yMin), &(limits->zMin)};
                        tokenPtr = strtok_chr(tmpPtr, ',');
                        for (unsigned int i = 0; i < 7; i++){
                            if (tokenPtr != NULL){
                                if (tmpArray[i] != NULL && !sscanf(tokenPtr, "%f", tmpArray[i])){
                                    tmpInt = 0; sscanf(tokenPtr, "%d", &tmpInt);
                                    *tmpArray[i] = (float)tmpInt;
                                }
                                tokenPtr = strtok_chr(NULL, ','); //next token
                            }
                        }
                        limits->xMax = limits->xMin + limits->xMax;
                        limits->yMax = limits->yMin + limits->yMax;
                        limits->zMax = limits->zMin + limits->zMax;
                        cvBlockDetected = true;

                        if (abs (0 - (limits->xMax + limits->yMax + limits->zMax + limits->xMin + limits->yMin + limits->zMin)) > NCPARSER_DETECT_MIN){
                            cvBlockDetectedValid = true;
                            limits->blockDetected = true;
                            if (debugOutput){fprintf(ncFileDebugHandle, "PARSER: CV: Stock detected : X-:%f X+:%f Y-:%f Y+:%f Z-:%f Z+:%f\n", limits->xMin, limits->xMax, limits->yMin, limits->yMax, limits->zMin, limits->zMax);}
                            break;
                        }
                        if (debugOutput){fprintf(ncFileDebugHandle, "PARSER: CV: invalid stock\n");}
                        break;
                    }
                }

                //comment: cutview light tool definition: {$comment} T{$tool.index} : {$tool.diameter} {$endcomment}
                tokenPtr = strrchr(strBuffer, 't');
                if (tokenPtr != NULL && *(tokenPtr+1) != ' '){
                    tmpPtr = strrchr(tokenPtr, ':');
                    if (tmpPtr != NULL && tmpPtr > tokenPtr){
                        float tmpFloat;
                        if (sscanf(tokenPtr, "t%d", &tmpInt) && sscanf(tmpPtr, ":%f", &tmpFloat)){
                            toolNum = tmpInt; if (toolNum + 1 > arrSizes->toolStrucLimit){toolNum--;}
                            tools[toolNum].num = toolNum; tools[toolNum].diameter = tmpFloat;
                            if(debugOutput){fprintf (ncFileDebugHandle, "PARSER: CV: Simple tool detected : T:%i, dia:%f\n", toolNum, tools[toolNum].diameter);}
                            break;
                        }
                    }
                }

                //comment: normal comment with a bit of cleanup
                if (!(Xfirst || Yfirst || Zfirst) && (comments[commentIndex].distWork + comments[commentIndex].distFast + comments[commentIndex].distCircular + comments[commentIndex].distDrill) > 0.001 && commentIndex + 1 < arrSizes->commentStrucLimit){commentIndex++;} //increment comment index number
                tokenPtr = strchr(strBuffer, '(');
                tmpPtr = strchr(strBuffer, ';');
                if (tokenPtr != NULL && tmpPtr != NULL){
                    if (tokenPtr > tmpPtr){tokenPtr = tmpPtr;}
                } else if (tokenPtr == NULL){tokenPtr = tmpPtr;}
                tokenPtr += (*(tokenPtr + 1) == ' ') ? 2 : 1; //skip comment character and leading space
                tmpPtr = strBuffer + strlen(strBuffer) - 1; //pointer to last character before \0
                if (tmpPtr > tokenPtr){
                    if (*tmpPtr == ')' || *tmpPtr == ' '){ //trim last parenthese or space
                        *tmpPtr-- = '\0';
                        if (*tmpPtr == ' '){*tmpPtr = '\0';} //trim last space
                    }
                }
                strncpy(comments[commentIndex].name, tokenPtr, NCPARSER_NAME_LEN - 1); //backup comment
                if (debugOutput){fprintf(ncFileDebugHandle, "COMMENT: \"%s\"\n", tokenPtr);}
                break;
            } else if (Gvalid){
                if (currentChar == 'x'){ //x
                    Xcurrent = tmpDouble + ((GflagCircular == 91) ? Xback : 0); //convert from relative to absolute if needed
                    if (Xfirst){
                        if (!cvBlockDetectedValid){limits->xMin = Xcurrent; limits->xMax = Xcurrent;}
                        for (unsigned int i = 0; i < lineIndex; i++){lines[i].x = Xcurrent;} //retro set coordinates
                        Xback = Xcurrent;
                        Xfirst = false;
                        goto GvalidParseEnd;
                    }

                    if (Xcurrent < limits->xMin){limits->xMin = Xcurrent;}
                    if (Xcurrent > limits->xMax){limits->xMax = Xcurrent;}
                    goto GvalidParseEnd;
                }
                
                if (currentChar == 'y'){ //y
                    Ycurrent = tmpDouble + ((GflagCircular == 91) ? Yback : 0); //convert from relative to absolute if needed
                    if (Yfirst){
                        if (!cvBlockDetectedValid){limits->yMin = Ycurrent; limits->yMax = Ycurrent;}
                        for (unsigned int i = 0; i < lineIndex; i++){lines[i].y = Ycurrent;} //retro set coordinates
                        Yback = Ycurrent;
                        Yfirst = false;
                        goto GvalidParseEnd;
                    }

                    if (Ycurrent < limits->yMin){limits->yMin = Ycurrent;}
                    if (Ycurrent > limits->yMax){limits->yMax = Ycurrent;}
                    goto GvalidParseEnd;
                }
                
                if (currentChar == 'z'){ //z
                    Zcurrent = tmpDouble + ((GflagCircular == 91) ? Zback : 0); //convert from relative to absolute if needed
                    if (Zfirst){
                        if (!cvBlockDetectedValid){
                            limits->zMin = Zcurrent;
                            limits->zMax = Zcurrent;
                            if (Gcurrent != 0){
                                limits->zMinWork = Zcurrent;
                                limits->zMaxWork = Zcurrent;
                            }
                        }
                        for (unsigned int i = 0; i < lineIndex; i++){lines[i].z = Zcurrent;} //retro set coordinates
                        Zback = Zcurrent;
                        Zfirst = false;
                        goto GvalidParseEnd;
                    }

                    if (Zcurrent < limits->zMin){limits->zMin = Zcurrent;}
                    if (Zcurrent > limits->zMax){limits->zMax = Zcurrent;}
                    if (Gcurrent != 0){
                        if (Zcurrent < limits->zMinWork){limits->zMinWork = Zcurrent;}
                        if (Zcurrent > limits->zMaxWork){limits->zMaxWork = Zcurrent;}
                    }
                    goto GvalidParseEnd;
                }
                
                if (currentChar == 'f'){ //speed
                    Fcurrent = ((unsigned int)tmpInt > maxSpeedXY) ? maxSpeedXY : tmpInt; //avoid overspeed
                    goto GvalidParseEnd;
                }
                
                if (currentChar == 'i'){ //circular: x center
                    Icurrent = tmpDouble + ((GflagCircular == 91) ? Xback : 0); //convert from relative to absolute if needed
                    Iset = true;
                    goto GvalidParseEnd;
                }
                
                if (currentChar == 'j'){ //circular: y center
                    Jcurrent = tmpDouble + ((GflagCircular == 91) ? Yback : 0); //convert from relative to absolute if needed
                    Jset = true;
                    goto GvalidParseEnd;
                }
                
                if (currentChar == 'k'){ //circular: z center (TO BE IMPLEMENTED)
                    Kcurrent = tmpDouble + ((GflagCircular == 91) ? Zback : 0); //convert from relative to absolute if needed
                    Kset = true;
                    goto GvalidParseEnd;
                }
                
                if (currentChar == 'q'){ //drill: cleaning depth (reversed)
                    Qcurrent = tmpDouble;
                    goto GvalidParseEnd;
                }
                
                if (currentChar == 'r'){ //drill: lift height
                    Rcurrent = tmpDouble;
                    goto GvalidParseEnd;
                }
                
                if (currentChar == 't'){  //tool change
                    toolNum = atoi(tokenPtr);
                    if(debugOutput){fprintf(ncFileDebugHandle, "PARSER: Tool change : T:%i\n",toolNum);}
                }
            }
            
            GvalidParseEnd:;
            tokenPtr = strtok_chr(NULL, ' '); //next token
        }

        //process line data
        if (!emptyLine){ //valid G function detected in current line
            lines[lineIndex].line = currentLine;
            lines[lineIndex].g = Gcurrent;
            lines[lineIndex].tool = toolNum;
            lines[lineIndex].comment = commentIndex;
            lines[lineIndex].f = (Gcurrent == 0) ? Ffast : Fcurrent;
            lines[lineIndex].x = Xcurrent; lines[lineIndex].y = Ycurrent; lines[lineIndex].z = Zcurrent;
            lines[lineIndex].i = Icurrent; lines[lineIndex].j = Jcurrent; lines[lineIndex].k = Kcurrent;
            lines[lineIndex].q = Qcurrent; lines[lineIndex].r = Rcurrent;

            if (debugOutput){fprintf(ncFileDebugHandle, "PARSER: g: %d, f: %d, x: %f, y: %f, z: %f, i: %f, j: %f, k: %f, q: %f, r: %f\n", lines[lineIndex].g, lines[lineIndex].f, lines[lineIndex].x, lines[lineIndex].y, lines[lineIndex].z, lines[lineIndex].i, lines[lineIndex].j, lines[lineIndex].k, lines[lineIndex].q, lines[lineIndex].r);}

            //compute distance traveled in x
            double tmpTravelX = numDiffDouble(Xback, Xcurrent);
            bool Xtraveled = (!Xfirst) ? (tmpTravelX > NCPARSER_DETECT_MIN) : false;

            //compute distance traveled in y
            double tmpTravelY = numDiffDouble(Yback, Ycurrent);
            bool Ytraveled = (!Yfirst) ? (tmpTravelY > NCPARSER_DETECT_MIN) : false;

            //compute distance traveled in z
            double tmpTravelZ = numDiffDouble(Zback, Zcurrent);
            bool Ztraveled = (!Zfirst) ? (tmpTravelZ > NCPARSER_DETECT_MIN) : false;

            int tmpInt = 0; double tmpDouble = 0., tmpTravel = 0.; //float tmpFloat = 0.f;

            if (Gcurrent == 0 || Gcurrent == 1){ //linear
                if (Xtraveled){tmpTravel = tmpTravelX;} //moved in x
                if (Ytraveled){tmpTravel = (tmpTravel > NCPARSER_DETECT_MIN) ? sqrt(tmpTravel*tmpTravel + tmpTravelY*tmpTravelY) : tmpTravelY;} //moved in y
                if (Ztraveled){tmpTravel = (tmpTravel > NCPARSER_DETECT_MIN) ? sqrt(tmpTravel*tmpTravel + tmpTravelZ*tmpTravelZ) : tmpTravelZ;} //moved in z

                if (tmpTravel > NCPARSER_DETECT_MIN){
                    if (Gcurrent == 0){ //fast
                        summary->g0++;
                        comments[commentIndex].distFast += tmpTravel;
                        comments[commentIndex].timeFast += tmpTravel / Ffast;
                        goto lineProcessingEnd;
                    }
                    
                    //work
                    summary->g1++;
                    comments[commentIndex].distWork += tmpTravel;
                    comments[commentIndex].timeWork += tmpTravel / Fcurrent;
                }
                goto lineProcessingEnd;
            }
            
            if (Gcurrent == 2 || Gcurrent == 3){ //circular
                int angleDir = 1; //G2-3 angle direction
                double angle = 0, angleStart = 0, angleEnd = 0; //angles for gd

                if (Gcurrent == 2){summary->g2++; angleDir = 1;} else {summary->g3++; angleDir = -1;} //increment line count and set angle direction
                if (Iset && Jset && (Xtraveled || Ytraveled || Ztraveled)){
                    tmpDouble = numDiffDouble(Icurrent, Xcurrent);
                    double tmpDouble1 = numDiffDouble(Jcurrent, Ycurrent);
                    Rcurrent = abs(sqrt(tmpDouble*tmpDouble + tmpDouble1*tmpDouble1));
                    angle = rad2deg(angle3points(Icurrent, Jcurrent, Xback, Yback, Xcurrent, Ycurrent));
                    angleStart = rad2deg(angle3points(Icurrent, Jcurrent, Icurrent + Rcurrent, Jcurrent, Xback, Yback));
                    angleEnd = rad2deg(angle3points(Icurrent, Jcurrent, Icurrent + Rcurrent, Jcurrent, Xcurrent, Ycurrent));

                    if(debugOutput){
                        if (numDiffDouble(angle, 0.) > 359.999 || numDiffDouble(angleStart, 0.) > 359.999 || numDiffDouble(angleEnd, 0.) > 359.999){
                            fprintf(ncFileDebugHandle, "BUG: Rcurrent:%lf, angle:%lf, startAngle:%lf, endAngle:%lf\n", Rcurrent, angle, angleStart, angleEnd);
                        }
                    }
                    
                    if (Gcurrent == 3){tmpDouble = angleStart; angleStart = angleEnd; angleEnd = tmpDouble;} //g03, reverse start and end angles

                    if ((Yback > Jcurrent && angleDir > 0) || (Ycurrent > Jcurrent && angleDir < 0)){
                        if ((Xback > Icurrent && angleDir > 0) || (Xcurrent > Icurrent && angleDir < 0)){angleStart = 359.999 - angleStart;} else {angleStart = 179.999 - angleStart + 179.999;}
                    }

                    if ((Ycurrent > Jcurrent && angleDir > 0) || (Yback - Jcurrent > 0 && angleDir < 0)){
                        if ((Xcurrent > Icurrent && angleDir > 0) || (Xback > Icurrent && angleDir < 0)){angleEnd = 359.999 - angleEnd;} else {angleEnd = 179.999 - angleEnd + 179.999;}
                    }

                    if (numDiffDouble(angleStart, 360.) < 0.001){angleStart = 0.001;}
                    if (numDiffDouble(angleEnd, 0.) < 0.001 && angleStart > 0){angleEnd = 359.999;}

                    if (angleStart + angle > 359.999 || angleStart > angleEnd){
                        //arc exceed limits
                        if ((Icurrent - Rcurrent) < limits->xMin || (Jcurrent - Rcurrent) < limits->yMin || (Icurrent + Rcurrent) > limits->xMax || (Jcurrent + Rcurrent) > limits->yMax){
                            double GcircleLimits[4] = {0,0,0,0};
                            arcLimits(GcircleLimits, Icurrent, Jcurrent, Rcurrent * 2, Rcurrent * 2, angleStart, 359.999, 1);
                            if (GcircleLimits[0] < limits->xMin){limits->xMin = GcircleLimits[0];}
                            if (GcircleLimits[1] > limits->xMax){limits->xMax = GcircleLimits[1];}
                            if (GcircleLimits[2] < limits->yMin){limits->yMin = GcircleLimits[2];}
                            if (GcircleLimits[3] > limits->yMax){limits->yMax = GcircleLimits[3];}
                        }
                        lines[lineIndex].startAngle = angleStart; lines[lineIndex].endAngle = 360.f;
                        angle = (360. - angleStart) + angleEnd; angleStart = 0.;
                    }

                    if (numDiffDouble(angleStart, 0.) < 0.001 && numDiffDouble(angleEnd, 360.) < 0.001) {angleEnd = 0.001;}

                    if (numDiffDouble(angleStart, angleEnd) > 0.001){
                        //arc exceed limits
                        if ((Icurrent - Rcurrent) < limits->xMin || (Jcurrent - Rcurrent) < limits->yMin || (Icurrent + Rcurrent) > limits->xMax || (Jcurrent + Rcurrent) > limits->yMax){
                            double GcircleLimits[4] = {0,0,0,0};
                            arcLimits(GcircleLimits, Icurrent, Jcurrent, Rcurrent * 2, Rcurrent * 2, angleStart, angleEnd, 1);
                            if (GcircleLimits[0] < limits->xMin){limits->xMin = GcircleLimits[0];}
                            if (GcircleLimits[1] > limits->xMax){limits->xMax = GcircleLimits[1];}
                            if (GcircleLimits[2] < limits->yMin){limits->yMin = GcircleLimits[2];}
                            if (GcircleLimits[3] > limits->yMax){limits->yMax = GcircleLimits[3];}
                        }
                        lines[lineIndex].startAngle1 = angleStart; lines[lineIndex].endAngle1 = angleEnd;
                    }

                    if(debugOutput){fprintf(ncFileDebugHandle, "PARSER: startAngle:%f, endAngle:%f, startAngle1:%f, endAngle1:%f, radius:%f, \n", lines[lineIndex].startAngle, lines[lineIndex].endAngle, lines[lineIndex].startAngle1, lines[lineIndex].endAngle1, lines[lineIndex].radius);}

                    lines[lineIndex].radius = Rcurrent;
                    tmpTravel = Rcurrent * deg2rad(angle); //travel distance
                    if (Ztraveled){tmpTravel = sqrt(tmpTravel*tmpTravel + tmpTravelZ*tmpTravelZ);} //incl z

                    if (tmpTravel > NCPARSER_DETECT_MIN){
                        comments[commentIndex].distCircular += tmpTravel;
                        comments[commentIndex].timeCircular += tmpTravel / Fcurrent;
                    }
                }
                goto lineProcessingEnd;
            }
            
            if (Gcurrent == 80){ //cancel drill operations
                Qcurrent = Rcurrent = drillRetractMode = 0;
                goto lineProcessingEnd;
            }

            if (Gcurrent == 98){ //drill: retract to initial z
                drillRetractMode = 1;
                drillZinit = Zcurrent;
                goto lineProcessingEnd;
            }
            
            if (Gcurrent == 81 || Gcurrent == 82 || Gcurrent == 83){ //drill
                summary->g81++; //increment line count

                if (Xtraveled){tmpTravel = tmpTravelX;} //moved in x
                if (Ytraveled){tmpTravel = (tmpTravel > NCPARSER_DETECT_MIN) ? sqrt(tmpTravel * tmpTravel + tmpTravelY * tmpTravelY) : tmpTravelY;} //moved in y

                tmpDouble = numDiffDouble(Zcurrent, Rcurrent); //total depth
                if (Qcurrent != 0){ //G83: with clearing
                    tmpInt = ceil (tmpDouble / Qcurrent); //steps
                    tmpTravelZ = 0; //reset z travel
                    for (int i = 0; i < (tmpInt + 1); i++){ //back and forth loop
                        tmpTravelZ += (i == tmpInt) ? (tmpDouble * 2) : (Qcurrent * i * 2); //last step/full depth, inbetween step
                    }
                } else { //G81: basic drilling
                    tmpTravelZ = tmpDouble * 2;
                }
                tmpTravel += tmpTravelZ; //add z travel to tool distance

                //retract to initial Z
                if (drillRetractMode == 1){ 
                    tmpTravel += numDiffDouble(drillZinit, Rcurrent);
                    lines[lineIndex].retractMode = drillRetractMode; 
                    lines[lineIndex].startAngle = drillZinit;
                }

                //update drill ops travel/time
                if (tmpTravel > NCPARSER_DETECT_MIN){
                    comments[commentIndex].distDrill += tmpTravel;
                    comments[commentIndex].timeDrill += (tmpTravel / Fcurrent);
                }
            }

            lineProcessingEnd:;
            if (tmpTravel > 0. && lineIndex + 1 < arrSizes->lineStrucLimit){lineIndex++;}
        }

        //backup current G/position
        Gback = Gcurrent;
        Xback = Xcurrent;
        Yback = Ycurrent;
        Zback = Zcurrent;
        Iset = Jset = Kset = false;
        currentLine++;
    }

    //set initial start
    if (lineIndex != 1){
        lines[0].g = 0;
        lines[0].x = lines[1].x;
        lines[0].y = lines[1].y;
        lines[0].z = lines[1].z;
    }

    //fill summary
    //tools
    unsigned int toolsCount = 0, toolLast = 0;
    float toolBiggest = 0.f;
    for (unsigned int i = 0; i < arrSizes->toolStrucLimit; i++){
        if (tools[i].num != -1){
            toolsCount++;
            toolLast = i;
            if (tools[i].diameter > toolBiggest){toolBiggest = tools[i].diameter;}
        }
    }
    limits->toolMax = toolBiggest;
    summary->tools = toolsCount;

    //lines/times/distances
    summary->all = /*summary->commented*/ + summary->skip + summary->g0 + summary->g1 + summary->g2 + summary->g3 + summary->g81;
    double distTotal = 0, timeTotal = 0, distFastTotal = 0, timeFastTotal = 0, distWorkTotal = 0, timeWorkTotal = 0, distCircularTotal = 0, timeCircularTotal = 0, distDrillTotal = 0, timeDrillTotal = 0;
    for (unsigned int i = 0; i < arrSizes->commentStrucLimit; i++){
        comments[i].distTotal = comments[i].distFast + comments[i].distWork + comments[i].distCircular + comments[i].distDrill;
        comments[i].timeTotal = comments[i].timeFast + comments[i].timeWork + comments[i].timeCircular + comments[i].timeDrill;
        distTotal += comments[i].distTotal; timeTotal += comments[i].timeTotal;
        distFastTotal += comments[i].distFast; timeFastTotal += comments[i].timeFast;
        distWorkTotal += comments[i].distWork; timeWorkTotal += comments[i].timeWork;
        distCircularTotal += comments[i].distCircular; timeCircularTotal += comments[i].timeCircular;
        distDrillTotal += comments[i].distDrill; timeDrillTotal += comments[i].timeDrill;
    }
    summary->distTotal = distTotal; summary->timeTotal = timeTotal;
    summary->distFastTotal = distFastTotal; summary->timeFastTotal = timeFastTotal;
    summary->distWorkTotal = distWorkTotal; summary->timeWorkTotal = timeWorkTotal;
    summary->distCircularTotal = distCircularTotal; summary->timeCircularTotal = timeCircularTotal;
    summary->distDrillTotal = distDrillTotal; summary->timeDrillTotal = timeDrillTotal;

    //array boundaries
    arrSizes->commentStrucLimit = commentIndex + 1;
    arrSizes->lineStrucLimit = lineIndex + 1;
    arrSizes->toolStrucLimit = toolLast + 1;

    if (debugOutput){ //debug
        fprintf(ncFileDebugHandle, "\n\n\nDEBUG REPORT\n");

        fprintf(ncFileDebugHandle, "\nstruct ncArraySize: var 'arrSizes'\n");
        fprintf(ncFileDebugHandle, "\tlineStrucLimit:%u ,commentStrucLimit:%u ,toolStrucLimit:%u\n", arrSizes->lineStrucLimit, arrSizes->commentStrucLimit, arrSizes->toolStrucLimit);

        fprintf(ncFileDebugHandle, "\nstruct ncToolStruc: var 'tools'\n");
        for (unsigned int i = 0; i < arrSizes->toolStrucLimit; i++){
            if (tools[i].num != -1){fprintf(ncFileDebugHandle, "\tnum:%d, diameter:%f, radius:%f, length:%f, angle:%f\n", tools[i].num, tools[i].diameter, tools[i].radius, tools[i].length, tools[i].angle);}
        }

        fprintf(ncFileDebugHandle, "\nstruct ncCommentStruc: var 'comments'\n");
        for (unsigned int i = 0; i < arrSizes->commentStrucLimit; i++){
            fprintf(ncFileDebugHandle, "\ti:%d, name:'%s', distTotal:%f, timeTotal:%f, distFast:%f, timeFast:%f, distWork:%f, timeWork:%f, distCircular:%f, timeCircular:%f, distDrill:%f, timeDrill:%f\n",
            i, comments[i].name, comments[i].distTotal, comments[i].timeTotal, comments[i].distFast, comments[i].timeFast, comments[i].distWork, comments[i].timeWork, comments[i].distCircular, comments[i].timeCircular, comments[i].distDrill, comments[i].timeDrill);
        }

        fprintf(ncFileDebugHandle, "\nstruct ncSummaryStruc: var 'summary'\n");
        fprintf(ncFileDebugHandle, "\tall:%u, commented:%u, skip:%u, g0:%u, g1:%u, g2:%u, g3:%u, g81:%u, tools:%u, distTotal:%f, timeTotal:%f, distFastTotal:%f, timeFastTotal:%f, distWorkTotal:%f, timeWorkTotal:%f, distCircularTotal:%f, timeCircularTotal:%f, distDrillTotal:%f, timeDrillTotal:%f\n", 
        summary->all, summary->commented, summary->skip, summary->g0, summary->g1, summary->g2, summary->g3, summary->g81, summary->tools, summary->distTotal, summary->timeTotal, summary->distFastTotal, summary->timeFastTotal, summary->distWorkTotal, summary->timeWorkTotal, summary->distCircularTotal, summary->timeCircularTotal, summary->distDrillTotal, summary->timeDrillTotal);

        fprintf(ncFileDebugHandle, "\nstruct ncLimitStruc: var 'limits'\n");
        fprintf(ncFileDebugHandle, "\txMin:%f, xMax:%f, yMin:%f, yMax:%f, zMin:%f, zMax:%f, zMinWork:%f, zMaxWork:%f\n", limits->xMin, limits->xMax, limits->yMin, limits->yMax, limits->zMin, limits->zMax, limits->zMinWork, limits->zMaxWork);

        fprintf(ncFileDebugHandle, "\nstruct ncFlagsStruc: var 'flags'\n");
        fprintf(ncFileDebugHandle, "\tcoord:%u, circular:%u, workplane:%u, compensation:%u, unit:%u\n", flags->coord, flags->circular, flags->workplane, flags->compensation, flags->unit);

        fprintf(ncFileDebugHandle, "\nstruct ncLineStruc: var 'lines'\n");
        for (unsigned int i = 0; i < arrSizes->lineStrucLimit; i++){
            fprintf(ncFileDebugHandle, "\ti:%u, line:%u, g:%d, tool:%d, comment:%u, f:%u, x:%f, y:%f, z:%f, i:%f, j:%f, k:%f, q:%f, r:%f, retractMode:%u, startAngle:%f, endAngle:%f, radius:%f, startAngle1:%f, endAngle1:%f\n",
            i, lines[i].line, lines[i].g, lines[i].tool, lines[i].comment, lines[i].f, lines[i].x, lines[i].y, lines[i].z, lines[i].i, lines[i].j, lines[i].k, lines[i].q, lines[i].r, lines[i].retractMode, lines[i].startAngle, lines[i].endAngle, lines[i].radius, lines[i].startAngle1, lines[i].endAngle1);
        }
    }

    //close streams
    if (debugOutput){
        fclose(ncFileDebugHandle);
        if (ncFileDebugBuffer != nullptr){delete []ncFileDebugBuffer;}
    }
    fclose(ncFileHandle); 

    if (arrSizes->lineStrucLimit == 1){return -ENOMSG;} //no data extracted
    debug = debugBack;
    return 0; //ok
}

void ncParseReportTerm(char* filename, unsigned int maxSpeedXY, unsigned int maxSpeedZ, unsigned int speedPercent, ncFlagsStruct *flags, ncToolStruct *tools, ncCommentStruct *comments, ncLimitStruct *limits, ncSummaryStruct *summary, ncArraySize *arrSizes){ //report gcode infos to terminal
    //default comment name if no comments
    char commentName[NCPARSER_NAME_LEN];
    if (arrSizes->commentStrucLimit == 1 && strlen(comments[0].name) == 0){
        strcpy(comments[0].name, strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_NC_NOCOMMENT][language]);
    }

    printfTerm("\n\n\033[0m");
    printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TITLE][language], filename);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_SPEED_XY][language], maxSpeedXY);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_SPEED_Z][language], maxSpeedZ);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_SETTINGS_TITLE][language]);
            printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_SETTINGS_COORD_LINE][language], flags->coord);
            printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_SETTINGS_COORD_CIRC][language], flags->circular);
            printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_SETTINGS_UNIT][language], flags->unit);
            if (flags->unit == 20){printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_SETTINGS_UNIT_NOTSUPP][language]);} else {printf("\n");}
            printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_SETTINGS_COMP][language], flags->compensation);
            if (flags->compensation != 40){printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_SETTINGS_COMP_NOTSUPP][language]);} else {printf("\n");}
            printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_SETTINGS_WORKPLANE][language], flags->workplane);
            if (flags->workplane != 17){printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_SETTINGS_WORKPLANE_NOTSUPP][language]);} else {printf("\n");}
    printf("\n\n");

    char timeArr[32];
    unsigned int commentLongest = 0;
    printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_TITLE][language]);
        sec2charArr(timeArr, summary->timeFastTotal * 60);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_FAST][language], timeArr);
        sec2charArr(timeArr, summary->timeWorkTotal * 60);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_WORK][language], timeArr);
        sec2charArr(timeArr, summary->timeCircularTotal * 60);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_CIRC][language], timeArr);
        sec2charArr(timeArr, summary->timeDrillTotal * 60);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_DRILL][language], timeArr);
        sec2charArr(timeArr, summary->timeTotal * 60);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_TOTAL][language], timeArr);
            //get longest comment to align output
            for (unsigned int i = 0; i < arrSizes->commentStrucLimit; i++){
                if (comments[i].distTotal > 0 || comments[i].timeTotal > 0){
                    UTF8toCP850(commentName, comments[i].name);
                    unsigned int tmpLen = strlen(commentName);
                    if (tmpLen > commentLongest){commentLongest = tmpLen;}
                }
            }

            //print sections
            for (unsigned int i = 0; i < arrSizes->commentStrucLimit; i++){
                if (comments[i].distTotal > 0 || comments[i].timeTotal > 0){
                    sec2charArr(timeArr, comments[i].timeTotal * 60);
                    UTF8toCP850(commentName, comments[i].name);
                    unsigned int tmpLen = commentLongest + 4 - strlen(commentName);
                    char tmpPad[tmpLen]; charArrFill(tmpPad, tmpLen, ' ');
                    printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_COMMENT_TITLE][language], commentName, tmpPad, timeArr);
                }
            }

    if (speedPercent != 100){
        printf("\n\n");
        double speedCorrection = (double)speedPercent / 100;
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_PERCENT_TITLE][language], speedPercent);
            sec2charArr(timeArr, (summary->timeFastTotal / speedCorrection) * 60);
            printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_FAST][language], timeArr);
            sec2charArr(timeArr, (summary->timeWorkTotal / speedCorrection) * 60);
            printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_WORK][language], timeArr);
            sec2charArr(timeArr, (summary->timeCircularTotal / speedCorrection) * 60);
            printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_CIRC][language], timeArr);
            sec2charArr(timeArr, (summary->timeDrillTotal / speedCorrection) * 60);
            printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_DRILL][language], timeArr);
            sec2charArr(timeArr, (summary->timeTotal / speedCorrection) * 60);
            printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_TOTAL][language], timeArr);
                for (unsigned int i = 0; i < arrSizes->commentStrucLimit; i++){
                    if (comments[i].distTotal > 0 || comments[i].timeTotal > 0){
                        sec2charArr(timeArr, (comments[i].timeTotal / speedCorrection) * 60);
                        UTF8toCP850(commentName, comments[i].name);
                        unsigned int tmpLen = commentLongest + 4 - strlen(commentName);
                        char tmpPad[tmpLen]; charArrFill(tmpPad, tmpLen, ' ');
                        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TIME_COMMENT_TITLE][language], commentName, tmpPad, timeArr);
                    }
                }
    }
        printf("\n\n");

        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_LIMITS_TITLE][language]);
            printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_LIMITS_x][language], "X", limits->xMin, limits->xMax, limits->xMax - limits->xMin);
            printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_LIMITS_x][language], "Y", limits->yMin, limits->yMax, limits->yMax - limits->yMin);
            printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_LIMITS_x][language], "Z", limits->zMin, limits->zMax, limits->zMax - limits->zMin);
        printf("\n\n");
        
    if (summary->tools > 0){
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TOOLS_TITLE][language]);
        for (unsigned int i = 0; i < arrSizes->toolStrucLimit; i++){
            if (tools[i].num != -1 && numDiffFloat(0.f, tools[i].diameter) > 0.001){
                printfTerm("\t\033[1;32mT%d\033[0m : \033[1;36mdia%.3fmm\033[0m", tools[i].num, tools[i].diameter); //Tnum : dia0.XXXmm
                if (numDiffFloat(0.f, tools[i].length) > 0.001f){
                    printfTerm(", \033[1;36mL%.1fmm\033[0m", tools[i].length); // , L0.Xmm
                }
                if (numDiffFloat(0.f, tools[i].radius) > 0.001f){
                    printfTerm(", \033[1;36mR%.1fmm\033[0m", tools[i].radius); // , R0.Xmm
                }
                if (numDiffFloat(0.f, tools[i].angle) > 0.001f){
                    printfTerm(", \033[1;36mV%.1fmm\033[0m", tools[i].angle); // , V0.Xmm
                }
                printf("\n");
            }
        }
        printf("\n\n");
    }

    printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TOOLPATHS_TITLE][language]);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TOOLPATHS_FAST][language], summary->distFastTotal);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TOOLPATHS_WORK][language], summary->distWorkTotal);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TOOLPATHS_CIRC][language], summary->distCircularTotal);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TOOLPATHS_DRILL][language], summary->distDrillTotal);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_TOOLPATHS_TOTAL][language], summary->distTotal);
    printf("\n\n");

    printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_GCODE_TITLE][language]);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_GCODE_TOL][language], summary->all);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_GCODE_COMMENTED][language], summary->commented);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_GCODE_IGNORED][language], summary->skip);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_GCODE_Gx][language], "0", summary->g0);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_GCODE_Gx][language], "1", summary->g1);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_GCODE_Gx][language], "2", summary->g2);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_GCODE_Gx][language], "3", summary->g3);
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_GCODE_DRILL][language], summary->g81);
    printf("\n\n");

    if (limits->zMin - limits->zMinWork < -0.1f){
        printfTerm(strParserReportTerm[STR_PARSER_REPORT::PARSER_REPORT_WARNING_TOOLCRASH][language], limits->zMin, limits->zMinWork);
        printf("\n\n");
    }

    printf("\033[0m");
}

