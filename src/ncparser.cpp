/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to gcode parsing.
*/

#include "ncparser.h"

bool inArrayInt (int *arr, int value, int arrSize) { //check if value in array
    for (int i=0; i < arrSize; i++) {if (arr[i] == value) {return true;}}
    return false;
}

void secToClock (char *arr, double value) {
    int hours = 0, minutes = 0, seconds = 0;
    hours = floor (value / 3600); minutes = floor ((value - (hours * 3600)) / 60); seconds = value - ((hours * 3600) + (minutes * 60));
    if (hours>0) {sprintf(arr, "%d:%02d:%02d", hours, minutes, seconds);} else {sprintf(arr, "%02d:%02d", minutes, seconds);}
}

unsigned int NCcountLines (char *file){
    unsigned int lineCount = 1; //line count var
    char tmpBuffer [4096];
    FILE* tmpHandle = fopen(file, "r"); //file stream
    if (tmpHandle != NULL) { //valid file stream
        while(fgets(tmpBuffer, 4095, tmpHandle)) {lineCount++;} //read lines
        fclose(tmpHandle); return lineCount; //close stream and return
    }
    return 0; //failed
}

double numDiffDouble (double n1, double n2) {if(n1 > n2){return abs(n2 - n1);}else{return abs(n1 - n2);}}
double rad2deg (double rad) {return rad * (180.0 / 3.14159265358979323846);}
double deg2rad (double deg) {return deg * (3.14159265358979323846 / 180.0);}
double angle3points(double centerX, double centerY, double x1, double y1, double x2, double y2){double v1x = x2 - centerX; double v1y = y2 - centerY; double v2x = x1 - centerX; double v2y = y1 - centerY; return acos ((v1x * v2x + v1y * v2y) / (sqrt (v1x*v1x + v1y*v1y) * sqrt (v2x*v2x + v2y*v2y)));}

void arcLimits(double *arr, double centerX, double centerY, double width, double height, double angleStart, double angleEnd, int resolution){
    if (resolution < 1){resolution = 20;}
    double xMin, xMax, yMin, yMax, x, y;
    double hwidth = width / 2; double hheight = height / 2;
    double angle1 = deg2rad(angleStart); double x1 = centerX + cos(angle1) * hwidth; double y1 = centerY - sin(angle1) * hheight;
    double angle2 = deg2rad(angleEnd); double x2 = centerX + cos(angle2) * hwidth; double y2 = centerY - sin(angle2) * hheight;

    if(x1 > x2){xMin = x2; xMax = x1;}else{xMin = x1; xMax = x2;}
    if(y1 > y2){yMin = y2; yMax = y1;}else{yMin = y1; yMax = y2;}

    double arcLen = (angle2 - angle1) * (height / 2);
    int step = ceil(arcLen / resolution);
    double stepangle = (angle2 - angle1) / step;

    for (int i = 1; i < (step + 1); i++){
        x = centerX + cos (angle1 + stepangle * i) * hwidth;
        y = centerY - sin (angle1 + stepangle * i) * hheight;
        if (x < xMin){xMin = x;}
        if (x > xMax){xMax = x;}
        if (y < yMin){yMin = y;}
        if (y > yMax){yMax = y;}
    }
    arr [0] = xMin; arr [1] = xMax; arr [2] = yMin; arr [3] = yMax;
}

int NCparseFile (char *file, ncFlagsStruc *flags, ncLineStruc *lines, ncToolStruc *tools, ncDistTimeStruc *ops, ncLimitStruc *limits, ncLinesCountStruc *linescount, ncArraySize *arrSizes, bool debugOutput) {
    FILE *ncFileHandle, *ncFileDebugHandle; //file handle
    int currLine = 1; //current line in file
    bool strFirstChar = true; //used to avoid things on first caracter
    char strBuffer [4096], strTmpBuffer [4096]; //buffer var
    char strFileDebug [PATH_MAX]; if(debugOutput){sprintf(strFileDebug, "%s%s", file, "_parserdebug.txt");}

    unsigned int commentIndex = 0; //current comment index
    //double commentTimeWork = 0., commentTimeFast = 0.;
    unsigned int toolNum = 0; //detected tool number

    char *tmpPtr, *tmpPtr1; //temp pointers
    int tmpInt = 0; float tmpFloat = 0.; double tmpDouble = 0., tmpDouble1 = 0.; //temp vars

    bool CVblockSet = false, nothingInLine = true;
    bool Xfirst = true, Yfirst = true, Zfirst = true; //first coordonate connected
    
    double Xnew = 0., Xlast = 0., Ynew = 0., Ylast = 0., Znew = 0., Zlast = 0., Inew = 0.,  Jnew = 0., /*Knew = 0., */Qnew = 0., Rnew = 0.; //new and last position
    int Fnew = lines->f, Ffast = lines->f; //new and last speed
    bool GnewSet = false, XnewSet = false, YnewSet = false, ZnewSet = false, InewSet = false, JnewSet = false, KnewSet = false, QnewSet = false, RnewSet = false, FnewSet = false;  //new set bools
    int Gnew = -1; //flag detected,current G function, last G function

    double tmpTravelX = 0, tmpTravelY = 0, tmpTravelZ = 0, tmpTravel = 0; //distance traveled in current line

    int GflagUnit = -1; /*int GflagUnitArr[] = {20,21}*/; //G20:inch (TO BE IMPLEMENTED), G21:mm
    int GflagPlane = -1; /*int GflagPlaneArr[] = {17,18,19}*/; //G17:XY, G18:ZX (TO BE IMPLEMENTED), G19:YZ (TO BE IMPLEMENTED)
    int GflagComp = -1; /*int GflagCompArr[] = {40,41,42}*/; //G40:none, G41:left , G42:right, Note: G41 and G42 will never be implemented because of the huge amount of work required
    int GflagCoord = -1, GflagCircular = -1; /*int GflagCoordArr[] = {90,91}*/; //linear: G90:absolute, G91:relative, circular: G90.1:absolute, G91.1:relative
    int GflagRemain = 5; //remaining G flags to be defined

    unsigned int arrLine = -1; //current line index (array)
    bool flagDetected = false; 
    //bool Gtype = true; //true: work, false:fast
    //int GlinearArr[] = {0,1}, GcircularArr[] = {2,3}, GdrillArr[] = {81,82,83,80}; //g function identifiers
    int GcompatibleArr[] = {0,1,2,3,80,81,82,83};

    //G2-3 specific
    int GangleDir = 1; //G2-3 angle direction
    double Gangle = 0, GangleStart = 0, GangleEnd = 0; //angles for gd
    double GcircleLimits [] = {0,0,0,0};
    bool arcOverLimits = false; //nns


    ncFileHandle = fopen(file, "r"); //file stream
    if (ncFileHandle != NULL) { //valid file stream
        if(debugOutput){ncFileDebugHandle = fopen(strFileDebug, "wb");}
        while(fgets(strBuffer, 4095, ncFileHandle)) { //read lines
            //clean current line
            strFirstChar = true; //"first" char of the line
            tmpPtr = strBuffer; tmpPtr1 = strTmpBuffer; //pointers
            if (currLine == 1 && strstr (strBuffer,"\xEF\xBB\xBF") != NULL) {tmpPtr += 3;} //remove utf8 bom
            while ((*tmpPtr != '\n' && *tmpPtr != '\0') && (*tmpPtr == ' ' || *tmpPtr == '\t')) {tmpPtr++;} //trim leading spaces
            while ((*tmpPtr != '\n' && *tmpPtr != '\0')) { //remove multiple spaces : based on https://stackoverflow.com/a/16790505
                *tmpPtr = tolower(*tmpPtr); //lowercase
                while (*tmpPtr == ' ' && *(tmpPtr + 1) == ' ') {tmpPtr++; *tmpPtr = tolower(*tmpPtr);} //skip multiple spaces
                if (strFirstChar != false) { //not the first char of the line
                    if ((*(tmpPtr - 1) == 'g' || *(tmpPtr - 1) == 'm') && *tmpPtr == '0' && *(tmpPtr + 1) != ' ') {tmpPtr++;} //skip 0 after functions
                    if (isdigit(*(tmpPtr + 1)) != 0 && *tmpPtr == ' ') {tmpPtr++;} //remove space before numbers
                    strFirstChar = true; //increment char position
                }
                *tmpPtr1++ = *tmpPtr++; //copy
            }
            if (*(tmpPtr1 - 1) == '\r') {tmpPtr1--;} //remove \r
            *tmpPtr1='\0'; strcpy(strBuffer, strTmpBuffer); //copy char array
            if(debugOutput){fprintf (ncFileDebugHandle, "%s\n",strBuffer);}

            //parse current line
            tmpPtr = strBuffer; //pointers
            if (*tmpPtr == '(' || *tmpPtr == ';') { //comments
                if (strstr (strBuffer,"tool/mill") != NULL) { //cutview tool definition: {$comment} TOOL/MILL,{$tool.diameter},{$tool.radius},{$tool.length},0 {$endcomment}
                    sscanf (strBuffer, "( tool/mill,%f,%f,%f,%f )", &tools[toolNum].diameter, &tools[toolNum].radius, &tools[toolNum].length, &tools[toolNum].angle);
                    if(debugOutput){fprintf (ncFileDebugHandle, "DEBUG: CV: Tool detected : T:%d, dia:%f, rad:%f, len:%f, ang:%f\n",toolNum,tools[toolNum].diameter,tools[toolNum].radius,tools[toolNum].length,tools[toolNum].angle);}
                } else if (strstr (strBuffer,"stock/block") != NULL) { //cutview block detection block: {$comment} STOCK/BLOCK,{$stock_width},{$stock_length},{$stock_height},{$stock_x},{$stock_y},{$stock_z} {$endcomment}
                    sscanf (strBuffer, "( stock/block,%lf,%lf,%lf,%lf,%lf,%lf )", &(limits[0].xMax), &(limits[0].yMax), &(limits[0].zMax), &(limits[0].xMin), &(limits[0].yMin), &(limits[0].zMin));
                    limits[0].xMax = limits[0].xMin + limits[0].xMax;
                    limits[0].yMax = limits[0].yMin + limits[0].yMax;
                    limits[0].zMax = limits[0].zMin + limits[0].zMax;
                    if(debugOutput){fprintf (ncFileDebugHandle, "DEBUG: CV: Stock detected : X-:%f X+:%f Y-:%f Y+:%f Z-:%f Z+:%f\n",limits[0].xMin,limits[0].xMax,limits[0].yMin,limits[0].yMax,limits[0].zMin,limits[0].zMax);}
                    if (abs (0 - (limits[0].xMax + limits[0].yMax + limits[0].zMax + limits[0].xMin + limits[0].yMin + limits[0].zMin)) > 0.) {
                        CVblockSet = true; if(debugOutput){fprintf (ncFileDebugHandle, "DEBUG: CV: valid stock\n");}
                    } else {if(debugOutput){fprintf (ncFileDebugHandle, "DEBUG: CV: invalid stock\n");}}
                } else {
                    if (sscanf (strBuffer, "( t%d :%f )", &tmpInt, &tmpFloat)) { //cutview light tool definition: {$comment} T{$tool.index} : {$tool.diameter} {$endcomment}
                        toolNum = tmpInt; if (toolNum + 1 > arrSizes->toolStrucLimit) {toolNum--;}
                        tools[toolNum].num = toolNum; tools[toolNum].diameter = tmpFloat;
                        if(debugOutput){fprintf (ncFileDebugHandle, "DEBUG: CV: Simple tool detected : T:%i, dia:%f\n",tmpInt,tmpFloat);}
                    } else {
                        if (ops[commentIndex].distWork > 0. && ops[commentIndex].distFast > 0. && commentIndex + 1 < arrSizes->distTimeStrucLimit) {commentIndex++;} //increment comment index number
                        strncpy (ops[commentIndex].name, strBuffer, 63); //backup comment

                        //printf("commentIndex:%d, distFast:%lf, distWork:%lf\n",commentIndex,ops[commentIndex].distFast,ops[commentIndex].distWork);

                        //ops[commentIndex].distWork = ops[commentIndex].distFast = ops[commentIndex].timeWork = ops[commentIndex].timeFast = 0;

                        //printf("commentIndex:%d, distFast:%lf, distWork:%lf\n",commentIndex,ops[commentIndex].distFast,ops[commentIndex].distWork);
                        //if (commentIndex > 0){ops[commentIndex - 1].fast = commentTimeFast; ops[commentIndex - 1].work = commentTimeWork;} //backup previous comment times
                        //commentTimeWork = 0., commentTimeFast = 0.; //reset comment times
                    }
                }
                linescount[0].commented++;
            } else if (*tmpPtr == 't') { //tool change
                toolNum = atoi (tmpPtr + 1);
                if(debugOutput){fprintf (ncFileDebugHandle, "DEBUG: Tool change : T:%i\n",toolNum);}
            } else {
                nothingInLine = true; //nothing detected in current line
                GnewSet = XnewSet = YnewSet = ZnewSet = InewSet = JnewSet = KnewSet = QnewSet = RnewSet = FnewSet = false; //reset set bool
                tmpPtr = strtok (strBuffer, " "); //split line
                while (tmpPtr != NULL) { //loop thru tokens
                    if (*tmpPtr == 'g') { //functions
                        Gnew = atoi (tmpPtr + 1); //backup code
                        flagDetected = false; //reset flag detected bool
                        if (GflagRemain != 0) { //some flags are not declared
                            if (GflagUnit == -1 && (Gnew == 20 || Gnew == 21)/*inArrayInt (GflagUnitArr, Gnew, 2)*/) { //G20:inch, G21:mm
                                GflagUnit = Gnew; flags->/*.*/unit = Gnew; GflagRemain--; flagDetected = true;
                                if(debugOutput){fprintf (ncFileDebugHandle, "DEBUG: Flag:unit:%i\n",Gnew);}
                            } else if (GflagPlane == -1 && (Gnew == 17 || Gnew == 18 || Gnew == 19)/*inArrayInt (GflagPlaneArr, Gnew, 3)*/) { //G17:XY, G18:ZX, G19:YZ
                                GflagPlane = Gnew; flags->/*.*/workplane = Gnew; GflagRemain--; flagDetected = true;
                                if(debugOutput){fprintf (ncFileDebugHandle, "DEBUG: Flag:plane:%i\n",Gnew);}
                            } else if (GflagComp == -1 && (Gnew == 40 || Gnew == 41 || Gnew == 42)/*inArrayInt (GflagCompArr, Gnew, 3)*/) { //G40:none, G41:left, G42:right
                                GflagComp = Gnew; flags->/*.*/compensation = Gnew; GflagRemain--; flagDetected = true;
                                if(debugOutput){fprintf (ncFileDebugHandle, "DEBUG: Flag:comp:%i\n",Gnew);}
                            } else if (GflagCircular == -1 && strchr(tmpPtr + 1, '.') != NULL && (Gnew == 90 || Gnew == 91)/*inArrayInt (GflagCoordArr, Gnew, 2)*/) { //circular: G90.1:absolute, G91.1:relative
                                GflagCircular = Gnew; flags->/*.*/circular = Gnew; GflagRemain--; flagDetected = true;
                                if(debugOutput){fprintf (ncFileDebugHandle, "DEBUG: Flag:circular:%i\n",Gnew);}
                            } else if (GflagCoord == -1 && (Gnew == 90 || Gnew == 91)/*inArrayInt (GflagCoordArr, Gnew, 2)*/) { //linear: G90:absolute, G91:relative
                                GflagCoord = Gnew; flags->/*.*/coord = Gnew; GflagRemain--; flagDetected = true;
                                if(debugOutput){fprintf (ncFileDebugHandle, "DEBUG: Flag:linear:%i\n",Gnew);}
                            }
                        }
                        if (!flagDetected) {
                            if (inArrayInt (GcompatibleArr, Gnew, 8)/*inArrayInt (GlinearArr, Gnew, 2) || inArrayInt (GcircularArr, Gnew, 2) || inArrayInt (GdrillArr, Gnew, 4)*/) { //valid g function
                                if (arrLine + 1 < arrSizes->lineStrucLimit) {arrLine++;}
                                //arrLine++;
                                lines[arrLine].g = Gnew;
                                lines[arrLine].tool = toolNum;
                                //if (Gnew == 0) {Gtype = false;} else {Gtype = true;} //fast or work bool
                                GnewSet = true;
                                nothingInLine = false;
                                if (Gnew >= 80 && Gnew < 83/*inArrayInt (GdrillArr, Gnew, 4)*/) {Qnew = Rnew = 0;} //reset q,r if drill by security
                            }
                        }
                    } else if (Gnew != -1 && !nothingInLine) { //G function already detected
                        tmpInt = atoi (tmpPtr + 1); tmpDouble = atof (tmpPtr + 1);
                        if (*tmpPtr == 'f') { //speed
                            if ((unsigned int) tmpInt > speedFastXY) {tmpInt = speedFastXY;} //avoid overspeed
                            lines[arrLine].f = tmpInt;
                            Fnew = tmpInt; FnewSet = true;
                        } else if (*tmpPtr == 'x') { //x
                            if (Xfirst) {if (!CVblockSet) {limits[0].xMin = tmpDouble; limits[0].xMax = tmpDouble;} Xfirst = false;}
                            if (tmpDouble < limits[0].xMin) {limits[0].xMin = tmpDouble;} if (tmpDouble > limits[0].xMax) {limits[0].xMax = tmpDouble;}
                            lines[arrLine].x = tmpDouble;
                            Xnew = tmpDouble; XnewSet = true;
                        } else if (*tmpPtr == 'y') { //y
                            if (Yfirst) {if (!CVblockSet) {limits[0].yMin = tmpDouble; limits[0].yMax = tmpDouble;} Yfirst = false;}
                            if (tmpDouble < limits[0].yMin) {limits[0].yMin = tmpDouble;} if (tmpDouble > limits[0].yMax) {limits[0].yMax = tmpDouble;}
                            lines[arrLine].y = tmpDouble;
                            Ynew = tmpDouble; YnewSet = true;
                        } else if (*tmpPtr == 'z') { //z
                            if (Zfirst) {if (!CVblockSet) {limits[0].zMin = tmpDouble; limits[0].zMax = tmpDouble;} Zfirst = false;}
                            if (tmpDouble < limits[0].zMin) {limits[0].zMin = tmpDouble; if (Gnew != 0) {limits[0].zMinWork = tmpDouble;}}
                            if (tmpDouble > limits[0].zMax) {limits[0].zMax = tmpDouble; if (Gnew != 0) {limits[0].zMaxWork = tmpDouble;}}
                            lines[arrLine].z = tmpDouble;
                            Znew = tmpDouble; ZnewSet = true;
                        } else if (*tmpPtr == 'i') {lines[arrLine].i = tmpDouble; Inew = tmpDouble;  InewSet = true; //circular: x center
                        } else if (*tmpPtr == 'j') {lines[arrLine].j = tmpDouble; Jnew = tmpDouble;  JnewSet = true; //circular: y center
                        //} else  if (*tmpPtr == 'k') {lines[arrLine].k = tmpDouble; Knew = tmpDouble;  KnewSet = true; //circular: z center (TO BE IMPLEMENTED)
                        } else  if (*tmpPtr == 'q') {lines[arrLine].q = tmpDouble; Qnew = tmpDouble; QnewSet = true; //drill: cleaning depth (reversed)
                        } else  if (*tmpPtr == 'r') {lines[arrLine].r = tmpDouble; Rnew = tmpDouble; RnewSet = true;} //drill: lift height
                    }
                    tmpPtr = strtok (NULL, " "); //next token
                }

                if (!nothingInLine) { //valid G function detected in current line
                    tmpTravel = 0;

                    //complet missing data
                    //if (Gnew != 80) {
                        if (!GnewSet) {lines[arrLine].g = Gnew;}
                        if (!XnewSet) {lines[arrLine].x = Xnew;}
                        if (!YnewSet) {lines[arrLine].y = Ynew;}
                        if (!ZnewSet) {lines[arrLine].z = Znew;}
                        if (!FnewSet) {lines[arrLine].f = Fnew;}
                        if (!QnewSet) {lines[arrLine].q = Qnew;}
                        if (!RnewSet) {lines[arrLine].r = Rnew;}
                    //} else {lines[arrLine].q = lines[arrLine].r = Qnew = Rnew = 0;} //cancel drill operations
                    if (Gnew == 0) {lines[arrLine].f = Ffast;
                    } else if (Gnew == 80) {lines[arrLine].q = lines[arrLine].r = Qnew = Rnew = 0;} //cancel drill operations

                    if(debugOutput){fprintf (ncFileDebugHandle, "DEBUG: g: %d, f: %d, x: %lf, y: %lf, z: %lf, i: %lf, j: %lf, k: %lf, q: %lf, r: %lf\n",lines[arrLine].g,lines[arrLine].f,lines[arrLine].x,lines[arrLine].y,lines[arrLine].z,lines[arrLine].i,lines[arrLine].j,lines[arrLine].k,lines[arrLine].q,lines[arrLine].r);}

                    if (GflagCoord==90) {tmpTravelX = numDiffDouble(Xlast, Xnew);} else {tmpTravelX = abs(Xnew);} //compute distance traveled in x
                    if (GflagCoord==90) {tmpTravelY = numDiffDouble(Ylast, Ynew);} else {tmpTravelY = abs(Ynew);} //compute distance traveled in y
                    if (GflagCoord==90) {tmpTravelZ = numDiffDouble(Zlast, Znew);} else {tmpTravelZ = abs(Znew);} //compute distance traveled in z

                    if (Gnew == 0 || Gnew == 1/*inArrayInt (GlinearArr, Gnew, 2)*/) { //linear
                        if (Gnew == 1) {linescount[0].g1++;} else {linescount[0].g0++;} //increment line count
                        if (tmpTravelX > 0.001 && tmpTravelY < 0.001) {tmpTravel = tmpTravelX; //only moved in x
                        } else if (tmpTravelX < 0.001 && tmpTravelY > 0.001) {tmpTravel = tmpTravelY; //only moved in y
                        } else if (tmpTravelX > 0.001 && tmpTravelY > 0.001) {tmpTravel = sqrt(tmpTravelX*tmpTravelX + tmpTravelY*tmpTravelY);} // moved both x y
                        if (tmpTravel > 0.001 && tmpTravelZ > 0.001) {tmpTravel = sqrt(tmpTravel*tmpTravel + tmpTravelZ*tmpTravelZ); //moved in x/y and z as well
                        } else if (tmpTravel < 0.001 && tmpTravelZ > 0.001) {tmpTravel = tmpTravelZ;} //only moved in z
                    } else if (Gnew == 2 || Gnew == 3/*inArrayInt (GcircularArr, Gnew, 2)*/) { //circular
                        if (Gnew == 2) {linescount[0].g2++; GangleDir = 1;} else {linescount[0].g3++; GangleDir = -1;} //increment line count and set angle direction
                        if (InewSet && JnewSet && (tmpTravelX > 0.001 || tmpTravelY > 0.001 || tmpTravelZ > 0.001)) {
                            if (GflagCircular == 91) {Inew = Xlast + Inew; Jnew = Ylast + Jnew;} //relative coordonates
                            tmpDouble = numDiffDouble(Inew, Xnew); tmpDouble1 = numDiffDouble(Jnew, Ynew);
                            Rnew = /*abs(*/sqrt (tmpDouble*tmpDouble + tmpDouble1*tmpDouble1)/*)*/;
                            Gangle = rad2deg (angle3points (Inew, Jnew, Xlast, Ylast, Xnew, Ynew));
                            GangleStart = rad2deg (angle3points (Inew, Jnew, Inew + Rnew, Jnew, Xlast, Ylast));
                            GangleEnd = rad2deg (angle3points (Inew, Jnew, Inew + Rnew, Jnew, Xnew, Ynew));

                            if(debugOutput){if (numDiffDouble (Gangle, 0.) > 360. || numDiffDouble (GangleStart, 0.) > 360. || numDiffDouble (GangleEnd, 0.) > 360.) {fprintf (ncFileDebugHandle, "BUG: Rnew:%lf, Gangle:%lf, startAngle:%lf, endAngle:%lf\n", Rnew, Gangle, GangleStart, GangleEnd);}}
                            
                            if (Gnew == 3) {tmpDouble = GangleStart; GangleStart = GangleEnd; GangleEnd = tmpDouble;} //g03, reverse start and end angles

                            if ((Ylast > Jnew && GangleDir > 0) || (Ynew > Jnew && GangleDir < 0)){
                                if ((Xlast > Inew && GangleDir > 0) || (Xnew > Inew && GangleDir < 0)){GangleStart = 360. - GangleStart;} else {GangleStart = 180. - GangleStart + 180.;}
                            }

                            if ((Ynew > Jnew && GangleDir > 0) || (Ylast - Jnew > 0 && GangleDir < 0)){
                                if ((Xnew > Inew && GangleDir > 0) || (Xlast > Inew && GangleDir < 0)){GangleEnd = 360. - GangleEnd;} else {GangleEnd = 180. - GangleEnd + 180.;}
                            }

                            if (numDiffDouble (GangleStart, 360.) < 0.01/*GangleStart == 360*/){GangleStart = 0.;}
                            if (numDiffDouble (GangleEnd, 0.) < 0.01/*GangleEnd == 0*/ && GangleStart > 0){GangleEnd = 360.;}

                            if (GangleStart + Gangle > 360. || GangleStart > GangleEnd) {
                                arcOverLimits = false;
                                if ((Inew - Rnew) < limits[0].xMin || (Jnew - Rnew) < limits[0].yMin || (Inew + Rnew) > limits[0].xMax || (Jnew + Rnew) > limits[0].yMax) {arcOverLimits = true;} //opt
                                if (arcOverLimits) {
                                    arcLimits (GcircleLimits, Inew, Jnew, Rnew * 2, Rnew * 2, GangleStart, 360, 10);
                                    if (GcircleLimits[0] < limits[0].xMin) {limits[0].xMin = GcircleLimits[0];}
                                    if (GcircleLimits[1] > limits[0].xMax) {limits[0].xMax = GcircleLimits[1];}
                                    if (GcircleLimits[2] < limits[0].yMin) {limits[0].yMin = GcircleLimits[2];}
                                    if (GcircleLimits[3] > limits[0].yMax) {limits[0].yMax = GcircleLimits[3];}
                                }
                                lines[arrLine].startAngle = GangleStart; lines[arrLine].endAngle = 360.; lines[arrLine].radius = Rnew;
                                Gangle = (360. - GangleStart) + GangleEnd; GangleStart = 0.;
                            }

                            if (numDiffDouble (GangleStart, 0.) < 0.01 && numDiffDouble (GangleEnd, 360.) < 0.01/*GangleStart == 0 && GangleEnd == 360*/) {GangleEnd = 0.;}

                            if (numDiffDouble (GangleStart, GangleEnd) > 0.01/*GangleStart != GangleEnd*/) {
                                arcOverLimits = false;
                                if ((Inew - Rnew) < limits[0].xMin || (Jnew - Rnew) < limits[0].yMin || (Inew + Rnew) > limits[0].xMax || (Jnew + Rnew) > limits[0].yMax) {arcOverLimits = true;} //opt
                                if (arcOverLimits) {
                                    arcLimits (GcircleLimits, Inew, Jnew, Rnew * 2, Rnew * 2, GangleStart, GangleEnd, 10);
                                    if (GcircleLimits[0] < limits[0].xMin) {limits[0].xMin = GcircleLimits[0];}
                                    if (GcircleLimits[1] > limits[0].xMax) {limits[0].xMax = GcircleLimits[1];}
                                    if (GcircleLimits[2] < limits[0].yMin) {limits[0].yMin = GcircleLimits[2];}
                                    if (GcircleLimits[3] > limits[0].yMax) {limits[0].yMax = GcircleLimits[3];}
                                }
                                lines[arrLine].startAngle1 = GangleStart; lines[arrLine].endAngle1 = GangleEnd; lines[arrLine].radius = Rnew;
                            }

                            if(debugOutput){fprintf (ncFileDebugHandle, "DEBUG: startAngle:%lf, endAngle:%lf, startAngle1:%lf, endAngle1:%lf, radius:%lf, \n", lines[arrLine].startAngle, lines[arrLine].endAngle, lines[arrLine].startAngle1, lines[arrLine].endAngle1, lines[arrLine].radius);}

                            tmpTravel = Rnew * deg2rad (Gangle); //travel distance
                            if (tmpTravelZ > 0.001) {tmpTravel = sqrt(tmpTravel*tmpTravel + tmpTravelZ*tmpTravelZ);} //incl z

                            if (tmpTravel > 0.001) {ops[commentIndex].distCircular += tmpTravel; ops[commentIndex].timeCircular += tmpTravel / Fnew;}
                        }
                    } else if (Gnew == 81 || Gnew == 82 || Gnew == 83/*inArrayInt (GdrillArr, Gnew, 4)*/) { //drill
                        linescount[0].g81++; //increment line count
                        if (tmpTravelX > 0.001 && tmpTravelY < 0.001) {tmpTravel = tmpTravelX; //only moved in x
                        } else if (tmpTravelX < 0.001 && tmpTravelY > 0.001) {tmpTravel = tmpTravelY; //only moved in y
                        } else if (tmpTravelX > 0.001 && tmpTravelY > 0.001) {tmpTravel = sqrt(tmpTravelX*tmpTravelX + tmpTravelY*tmpTravelY);} // moved both x y

                        tmpDouble = numDiffDouble(Znew, Rnew); //total depth
                        if (Qnew != 0) { //G83: with clearing
                            tmpInt = ceil (tmpDouble / Qnew); //steps
                            tmpTravelZ = 0; //reset z travel
                            for (int i = 0; i < (tmpInt + 1); i++){ //back and forth loop
                                if (i==tmpInt) {tmpTravelZ += tmpDouble * 2; //last step, full depth
                                } else {tmpTravelZ += Qnew * i * 2;} //inbetween step
                            }
                        } else {tmpTravelZ = tmpDouble * 2;} //G81: basic drilling
                        tmpTravel += tmpTravelZ; //add z travel to tool distance
                        if (tmpTravel > 0.001) {ops[commentIndex].distDrill += tmpTravel; ops[commentIndex].timeDrill += (tmpTravel / Fnew);} //update drill ops travel/time
                    }

                    if (tmpTravel > 0.001/* && (Gnew == 0 || Gnew == 1)*/) { //tool moved, update travel/time
                        if (Gnew == 0) {ops[commentIndex].distFast += tmpTravel; ops[commentIndex].timeFast += tmpTravel / Ffast; //fast
                        } else {ops[commentIndex].distWork += tmpTravel; ops[commentIndex].timeWork += tmpTravel / Fnew;} //work
                    }
                } else {linescount[0].skip++;}
            }
            
            /*lastGnew = Gnew;*/

            Xlast = Xnew;
            Ylast = Ynew;
            Zlast = Znew;
            currLine++;
        }

        linescount[0].all = linescount[0].commented + linescount[0].skip + linescount[0].g0 + linescount[0].g1 + linescount[0].g2 + linescount[0].g3 + linescount[0].g81; //compute line count
        tmpInt = 0; for (unsigned int i=0; i<arrSizes->toolStrucLimit; i++) {if (tools[i].num != -1){tmpInt++;}} linescount[0].tools = tmpInt; //amount of tools detected
        tmpDouble = 0; tmpDouble1 = 0; for (unsigned int i=0; i<arrSizes->distTimeStrucLimit; i++) {if (ops[i].timeFast > 0.){tmpDouble += ops[i].timeFast;} if (ops[i].timeWork > 0.){tmpDouble1 += ops[i].timeWork;}} linescount[0].totalTimeFast = tmpDouble; linescount[0].totalTimeWork = tmpDouble1; //compute total time

        //debug
        if(debugOutput){
            unsigned int i=0;
            double debugDistWork = 0., debugTimeWork = 0.;
            double debugDistFast = 0., debugTimeFast = 0.;
            double debugDistCircular = 0., debugTimeCircular = 0.;
            double debugDistDrill = 0., debugTimeDrill = 0.;
            char debugTimeWorkArr [] = "0000:00:00", debugTimeFastArr [] = "0000:00:00", debugTimeCircularArr [] = "0000:00:00", debugTimeDrillArr [] = "0000:00:00";
            fprintf (ncFileDebugHandle, "\n\n\nDEBUG REPORT\n");

            fprintf (ncFileDebugHandle, "\nstruct ncFlagsStruc: var 'flags'\n");
            //fprintf (ncFileDebugHandle, "\tcoord:%d, circular:%d, workplane:%d, compensation:%d, unit:%d\n",flags->coord,flags->circular,flags->workplane,flags->compensation,flags->unit);
            fprintf (ncFileDebugHandle, "\tcoord:%d, circular:%d, workplane:%d, compensation:%d, unit:%d\n",flags[0].coord,flags[0].circular,flags[0].workplane,flags[0].compensation,flags[0].unit);

            fprintf (ncFileDebugHandle, "\nstruct ncLimitStruc: var 'limits'\n");
            fprintf (ncFileDebugHandle, "\txMin:%lf, xMax:%lf, yMin:%lf, yMax:%lf, zMin:%lf, zMax:%lf, zMinWork:%lf, zMaxWork:%lf\n",limits[0].xMin,limits[0].xMax,limits[0].yMin,limits[0].yMax,limits[0].zMin,limits[0].zMax,limits[0].zMinWork,limits[0].zMaxWork);

            fprintf (ncFileDebugHandle, "\nstruct ncLinesCountStruc: var 'linescount'\n");
            fprintf (ncFileDebugHandle, "\tall:%d, commented:%d, skip:%d, g0:%d, g1:%d, g2:%d, g3:%d, g81:%d\n",linescount[0].all,linescount[0].commented,linescount[0].skip,linescount[0].g0,linescount[0].g1,linescount[0].g2,linescount[0].g3,linescount[0].g81);

            fprintf (ncFileDebugHandle, "\nstruct ncDistTimeStruc: var 'ops'\n");
            for (i=0; i<arrSizes->distTimeStrucLimit; i++) {
                if (ops[i].distWork>0. || ops[i].distFast>0.){
                    fprintf (ncFileDebugHandle, "\tname:'%s', distWork:%lf, distFast:%lf, distCircular:%lf, distDrill:%lf, timeWork:%lf, timeFast:%lf, timeCircular:%lf, timeDrill:%lf\n",ops[i].name,ops[i].distWork,ops[i].distFast,ops[i].distCircular,ops[i].distDrill,ops[i].timeWork,ops[i].timeFast,ops[i].timeCircular,ops[i].timeDrill);
                    debugDistWork += ops[i].distWork; debugTimeWork += ops[i].timeWork;
                    debugDistFast += ops[i].distFast; debugTimeFast += ops[i].timeFast;
                    debugDistCircular += ops[i].distCircular; debugTimeCircular += ops[i].timeCircular;
                    debugDistDrill += ops[i].distDrill; debugTimeDrill += ops[i].timeDrill;
                }
            }
            secToClock (debugTimeWorkArr, debugTimeWork*60); secToClock (debugTimeFastArr, debugTimeFast*60); secToClock (debugTimeCircularArr, debugTimeCircular*60); secToClock (debugTimeDrillArr, debugTimeDrill*60);
            fprintf (ncFileDebugHandle, "\tTotal, distWork: %lfmm, distFast: %lfmm, distCircular: %lfmm, distDrill: %lfmm\n",debugDistWork,debugDistFast,debugDistCircular,debugDistDrill);
            fprintf (ncFileDebugHandle, "\tTotal, timeWork: %s, timeFast: %s, timeCircular: %s, timeDrill: %s\n",debugTimeWorkArr,debugTimeFastArr,debugTimeCircularArr,debugTimeDrillArr);
            fprintf (ncFileDebugHandle, "\nstruct ncToolStruc: var 'tools'\n");
            for (i=0; i<arrSizes->toolStrucLimit; i++) {if (tools[i].num != -1){fprintf (ncFileDebugHandle, "\tnum:%d, diameter:%f, radius:%f, length:%f, angle:%f\n",tools[i].num,tools[i].diameter,tools[i].radius,tools[i].length,tools[i].angle);}}

            fprintf (ncFileDebugHandle, "\nstruct ncLineStruc: var 'lines'\n"); i=0;
            while (lines[i].g!=-1) {fprintf (ncFileDebugHandle, "\tline %d : g:%d, tool:%d, f:%d, x:%lf, y:%lf, z:%lf, i=%lf, j:%lf, k:%lf, q:%lf, r:%lf, startAngle:%lf, endAngle:%lf, startAngle1:%lf, endAngle1:%lf, radius:%lf\n",i,lines[i].g,lines[i].tool,lines[i].f,lines[i].x,lines[i].y,lines[i].z,lines[i].i,lines[i].j,lines[i].k,lines[i].q,lines[i].r,lines[i].startAngle,lines[i].endAngle,lines[i].startAngle1,lines[i].endAngle1,lines[i].radius); i++;}
            fclose(ncFileDebugHandle);
        }

        fclose(ncFileHandle); //close stream
    } else {return 1;} //ERR1 : fail to open
    return 0; //ok
}
