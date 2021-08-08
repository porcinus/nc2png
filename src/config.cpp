/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to configuration functions/vars.
*/

#include "config.h"


int inCharArray (char **arr, char *value, unsigned int arrSize) { //search in char array, return index if found, -1 if not
    for (unsigned int i = 0; i < arrSize; i++) {if (strcmp (arr[i], value) == 0) {return i;}}
    return -1;
}

bool configSave (bool reset, bool noOutput) { //save config file
    FILE *filehandle = fopen("nc2png.cfg", "wb");
    if (filehandle != NULL) {
        for (unsigned int i = 0; i < cfgVarsArrSize; i++) {
            int tmpType = cfgVarsType[i];
            if (tmpType == 0) {fprintf (filehandle, reset ? "" : "%s=%d;", cfgVarsName[i], *(int*)cfgVarsPtr[i]); //int
            } else if (tmpType == 1) {fprintf (filehandle, reset ? "" : "%s=%u;", cfgVarsName[i], *(unsigned int*)cfgVarsPtr[i]); //unsigned int
            } else if (tmpType == 2) {fprintf (filehandle, reset ? "" : "%s=%f;", cfgVarsName[i], *(float*)cfgVarsPtr[i]); //float
            } else if (tmpType == 3) {fprintf (filehandle, reset ? "" : "%s=%lf;", cfgVarsName[i], *(double*)cfgVarsPtr[i]);} //double
        }
        fclose(filehandle);
        printf (reset ? strConfig[language][STR_CONFIG::RESET_SUCCESS] : strConfig[language][STR_CONFIG::SAVE_SUCCESS]);
        printf ("\n");
        return true;
    } else {printf (strConfig[language][STR_CONFIG::WRITE_FAILED]);}
    return false;
}

void configParse (void) { //parse/create program config file
    char strBuffer [4096]; //string buffer
    FILE *filehandle = fopen("nc2png.cfg", "r");
    if (filehandle != NULL) {
        char *tmpPtr, *tmpEqPtr; //pointers
        char strDebugBuffer [4096] = "DEBUG: Config: ";
        while (fgets (strBuffer, 4095, filehandle) != NULL) { //line loop
            tmpPtr = strtok (strBuffer, ";"); //split element
            while (tmpPtr != NULL) { //var=val loop
                char strElementBuffer [strlen(tmpPtr)]; strcpy (strElementBuffer, tmpPtr); //copy element to new buffer to avoid pointer mess
                tmpEqPtr = strchr(strElementBuffer, '='); //'=' char position
                if (tmpEqPtr != NULL) { //contain '='
                    *tmpEqPtr='\0'; int tmpVarSize = strlen(strElementBuffer), tmpValSize = strlen(tmpEqPtr+1); //var and val sizes
                    char tmpVar [tmpVarSize]; strcpy (tmpVar, strElementBuffer); //extract var
                    char tmpVal [tmpValSize]; strcpy (tmpVal, tmpEqPtr + 1); //extract val
                    int tmpIndex = inCharArray ((char**)cfgVarsName, tmpVar, cfgVarsArrSize); //var in config array
                    if (tmpIndex != -1) { //found in config array
                        int tmpType = cfgVarsType[tmpIndex];
                        if (tmpType == 0) {*(int*)cfgVarsPtr[tmpIndex] = atoi (tmpVal); //int
                        } else if (tmpType == 1) {*(unsigned int*)cfgVarsPtr[tmpIndex] = atoi (tmpVal); //unsigned int
                        } else if (tmpType == 2) {*(float*)cfgVarsPtr[tmpIndex] = atof (tmpVal); //float
                        } else if (tmpType == 3) {*(double*)cfgVarsPtr[tmpIndex] = atof (tmpVal);} //double
                        if(debug){char strDebugBuffer1 [4096]; sprintf (strDebugBuffer1, "%s=%s", tmpVar, tmpVal); strcat (strDebugBuffer, strDebugBuffer1);}
                    }
                }
                tmpPtr = strtok (NULL, ";"); //next element
            }
        }
        fclose(filehandle);
        if(debug){fprintf(stderr, strDebugBuffer);}
    } else {configEdit (true);}
}

void configEdit (bool first) { //edit config file
    char strBuffer [100]; //string buffer
    printf(strConfig[language][STR_CONFIG::SET_NEW_SETTINGS]);
    char strBufferNewVals [4096] = ""; //string buffer
    for (unsigned int i = 0; i < cfgVarsArrSize; i++) {
        int tmpType = cfgVarsType[i];
        if (tmpType == 0) { //int
            printf (strConfigVars[language][i], *(int*)cfgVarsPtr[i]);
            int tmpnum = 0; fgets (strBuffer , 10 , stdin);
            if (strlen(strBuffer) > 1) {tmpnum = atoi (strBuffer); *(int*)cfgVarsPtr[i] = tmpnum;}
            char tmpBuffer[4094]; sprintf (tmpBuffer, strConfigVarsOut[language][i], *(int*)cfgVarsPtr[i]); strcat (strBufferNewVals, tmpBuffer);
        } else if (tmpType == 1) { //unsigned int
            printf (strConfigVars[language][i], *(unsigned int*)cfgVarsPtr[i]);
            unsigned int tmpnum = 0; fgets (strBuffer , 10 , stdin);
            if (strlen(strBuffer) > 1) {tmpnum = (unsigned int)atoi (strBuffer); *(unsigned int*)cfgVarsPtr[i] = tmpnum;}
            char tmpBuffer[4094]; sprintf (tmpBuffer, strConfigVarsOut[language][i], *(unsigned int*)cfgVarsPtr[i]); strcat (strBufferNewVals, tmpBuffer);
        } else if (tmpType == 2) { //float
            printf (strConfigVars[language][i], *(float*)cfgVarsPtr[i]);
            float tmpnum = 0; fgets (strBuffer , 10 , stdin);
            if (strlen(strBuffer) > 1) {tmpnum = (float)atof (strBuffer); *(float*)cfgVarsPtr[i] = tmpnum;}
            char tmpBuffer[4094]; sprintf (tmpBuffer, strConfigVarsOut[language][i], *(float*)cfgVarsPtr[i]); strcat (strBufferNewVals, tmpBuffer);
        } else if (tmpType == 3) { //double
            printf (strConfigVars[language][i], *(double*)cfgVarsPtr[i]);
            double tmpnum = 0; fgets (strBuffer , 10 , stdin);
            if (strlen(strBuffer) > 1) {tmpnum = atof (strBuffer); *(double*)cfgVarsPtr[i] = tmpnum;}
            char tmpBuffer[4094]; sprintf (tmpBuffer, strConfigVarsOut[language][i], *(double*)cfgVarsPtr[i]); strcat (strBufferNewVals, tmpBuffer);
        }
    }
    printf ("\033[0m");
    printf (strConfig[language][STR_CONFIG::NEW_SETTINGS]);
    printf (strBufferNewVals);
    printf (strConfig[language][STR_CONFIG::SAVE_CONFIRM]);
    printf ("\n");
    fgets (strBuffer , 10 , stdin);

    configSave (false);
}

void configManu (void) { //onfly config edit without save
    char strBuffer [100]; //string buffer
    for (unsigned int i = 0; i < sizeof(cfgVarsManuArr) / sizeof(*cfgVarsManuArr); i++) {
        int tmpIndex = cfgVarsManuArr[i];
        int tmpType = cfgVarsType[tmpIndex];
        if (tmpType == 0) { //int
            printf (strConfigVars[language][tmpIndex], *(int*)cfgVarsPtr[tmpIndex]);
            int tmpnum = 0; fgets (strBuffer , 10 , stdin);
            if (strlen(strBuffer) > 1) {tmpnum = atoi (strBuffer); *(int*)cfgVarsPtr[tmpIndex] = tmpnum;}
        } else if (tmpType == 1) { //unsigned int
            printf (strConfigVars[language][tmpIndex], *(unsigned int*)cfgVarsPtr[tmpIndex]);
            unsigned int tmpnum = 0; fgets (strBuffer , 10 , stdin);
            if (strlen(strBuffer) > 1) {tmpnum = (unsigned int)atoi (strBuffer); *(unsigned int*)cfgVarsPtr[tmpIndex] = tmpnum;}
        } else if (tmpType == 2) { //float
            printf (strConfigVars[language][tmpIndex], *(float*)cfgVarsPtr[tmpIndex]);
            float tmpnum = 0; fgets (strBuffer , 10 , stdin);
            if (strlen(strBuffer) > 1) {tmpnum = (float)atof (strBuffer); *(float*)cfgVarsPtr[tmpIndex] = tmpnum;}
        } else if (tmpType == 3) { //double
            printf (strConfigVars[language][tmpIndex], *(double*)cfgVarsPtr[tmpIndex]);
            double tmpnum = 0; fgets (strBuffer , 10 , stdin);
            if (strlen(strBuffer) > 1) {tmpnum = atof (strBuffer); *(double*)cfgVarsPtr[tmpIndex] = tmpnum;}
        }
    }
    printf ("\033[0m");
}


