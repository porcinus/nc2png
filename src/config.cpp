/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to configuration functions/vars.
*/

#include "config.h"

bool configSave (bool reset) { //save config file
    FILE *filehandle = fopen("nc2png.cfg", "wb");
    if (filehandle != NULL) {
        char strBuffer [256]={'\0'}; //string buffer
        for (unsigned int i = 0; i < configArrSize; i++) {strcat (strBuffer,"%d;");} //build format string
        fprintf (filehandle, reset ? "" : strBuffer, speedFastXY, speedFastZ, speedPercent, gdWidth, gdArcRes);
        fclose(filehandle);
        printf (reset ? strConfig[language][STR_CONFIG::RESET_SUCCESS] : strConfig[language][STR_CONFIG::SAVE_SUCCESS]);
        printf ("\n");
        return true;
    } else {printf (strConfig[language][STR_CONFIG::WRITE_FAILED]);}
    return false;
}

void configParse (void) { //parse/create program config file
    int tmpnum = 0; //store temporary input num to check
    char strBuffer [256]; //string buffer
    FILE *filehandle = fopen("nc2png.cfg", "r");
    if (filehandle != NULL) {
        if (fgets (strBuffer, 255, filehandle) != NULL) {
            unsigned int i = 0; char *tmpPtr; tmpPtr = strtok (strBuffer, ";");
            while (tmpPtr != NULL && i < configArrSize) {
                tmpnum = atoi (tmpPtr);
                if (tmpnum > 0) {*configArr[i] = tmpnum; tmpnum=0;}
                tmpPtr = strtok (NULL, ";"); i++;
            }
        }
        fclose(filehandle);
        if(debug){fprintf(stderr,"DEBUG: Config: speedFastXY=%d speedFastZ=%d speedPercent=%d gdWidth=%d gdArcRes=%d\n", speedFastXY, speedFastZ, speedPercent, gdWidth, gdArcRes);}
    } else {configEdit (true);/*configSave (false);*/}
}

void configEdit (bool first) { //edit config file
    int tmpnum = 0; //store temporary input num to check
    char strBuffer [100]; //string buffer
    printf(strConfig[language][STR_CONFIG::SET_NEW_SETTINGS]);

    printf (strConfig[language][STR_CONFIG::SET_SPEED_XY], speedFastXY);
    fgets (strBuffer , 10 , stdin); tmpnum = atoi (strBuffer); if (tmpnum > 0) {speedFastXY = tmpnum; tmpnum=0;}

    printf (strConfig[language][STR_CONFIG::SET_SPEED_Z], speedFastZ);
    fgets (strBuffer , 10 , stdin); tmpnum = atoi (strBuffer); if (tmpnum > 0) {speedFastZ = tmpnum; tmpnum=0;}

    printf (strConfig[language][STR_CONFIG::SET_SPEED_PERC], speedPercent);
    fgets (strBuffer , 10 , stdin); tmpnum = atoi (strBuffer); if (tmpnum > 0) {speedPercent = tmpnum; tmpnum=0;}

    printf (strConfig[language][STR_CONFIG::SET_GRIDSIZE], gdWidth);
    fgets (strBuffer , 10 , stdin); tmpnum = atoi (strBuffer); if (tmpnum > 0) {gdWidth = tmpnum; tmpnum=0;}

    printf (strConfig[language][STR_CONFIG::SET_ARCRES], gdArcRes);
    fgets (strBuffer , 10 , stdin); tmpnum = atoi (strBuffer); if (tmpnum > 0) {gdArcRes = tmpnum; tmpnum=0;}
    
    printf ("\033[0m");
    printf (strConfig[language][STR_CONFIG::NEW_SETTINGS], speedFastXY, speedFastZ, speedPercent, gdWidth, gdArcRes);
    printf (strConfig[language][STR_CONFIG::SAVE_CONFIRM]);
    printf ("\n");
    fgets (strBuffer , 10 , stdin);

    configSave (false);
}

void configManu (void) { //onfly config edit without save
    int tmpnum = 0; //store temporary input num to check
    char strBuffer [100]; //string buffer
    printf (strConfig[language][STR_CONFIG::SET_SPEED_XY], speedFastXY);
    fgets (strBuffer , 10 , stdin); tmpnum = atoi (strBuffer); if (tmpnum > 0) {speedFastXY = tmpnum; tmpnum=0;}
    printf (strConfig[language][STR_CONFIG::SET_SPEED_Z], speedFastZ);
    fgets (strBuffer , 10 , stdin); tmpnum = atoi (strBuffer); if (tmpnum > 0) {speedFastZ = tmpnum; tmpnum=0;}
    printf (strConfig[language][STR_CONFIG::SET_SPEED_PERC], speedPercent);
    fgets (strBuffer , 10 , stdin); tmpnum = atoi (strBuffer); if (tmpnum > 0) {speedPercent = tmpnum;}
    printf ("\033[0m");
}


