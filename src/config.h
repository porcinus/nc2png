/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to configuration functions/vars.
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <cstdlib>
#include <cstring> //string

//enum LANG_ID {LANG_EN, LANG_FR} ;
enum STR_CONFIG {RESET_SUCCESS,SAVE_SUCCESS,WRITE_FAILED,SET_NEW_SETTINGS,SET_SPEED_XY,SET_SPEED_Z,SET_SPEED_PERC,SET_GRIDSIZE,SET_ARCRES,NEW_SETTINGS,SAVE_CONFIRM};

const char *strConfigEN [] = {
/*RESET_SUCCESS*/ "\033[1;32mConfig file reset with success\033[0m\n",
/*SAVE_SUCCESS*/ "\033[1;32mConfig saved successfully\033[0m\n",
/*WRITE_FAILED*/ "\033[1;33mError: Failed to write config file\033[0m\n",
/*SET_NEW_SETTINGS*/ "\033[1mSet new settings (leave blank to keep unchanged)\033[0m\n",
/*SET_SPEED_XY*/ "\033[0mMaximum X and Y fast speed in mm/min (\033[1;33m%d\033[0m): \n > \033[1;32m",
/*SET_SPEED_Z*/ "\033[0mMaximum Z fast speed in mm/min (\033[1;33m%d\033[0m): \n > \033[1;32m",
/*SET_SPEED_PERC*/ "\033[0mSpeed override in percent (\033[1;33m%d\033[0m): \n > \033[1;32m ",
/*SET_GRIDSIZE*/ "\033[0mPreview grid max width/height in pixels (\033[1;33m%d\033[0m): \n > \033[1;32m ",
/*SET_ARCRES*/ "\033[0mPreview arc resolution in pixels (\033[1;33m%d\033[0m): \n > \033[1;32m ",
/*NEW_SETTINGS*/ "\nNew settings:\nXY: \033[1;32m%dmm/min\033[0m\nZ: \033[1;32m%dmm/min\033[0m\nSpeed: \033[1;32m%d%%\033[0m\nGrid: \033[1;32m%dpx\033[0m\nArc: \033[1;32m%dpx\033[0m\n\n",
/*SAVE_CONFIRM*/ "Press \033[1;32mEnter\033[0m to \033[1mSave\033[0m, \033[1;32mCtrl+C\033[0m to \033[1mCancel\033[0m"
};

const char *strConfigFR [] = {
/*RESET_SUCCESS*/ "\033[1;32mConfiguration reinitialisee avec succes\033[0m\n",
/*SAVE_SUCCESS*/ "\033[1;32mConfiguration sauvegardee avec succes\033[0m\n",
/*WRITE_FAILED*/ "\033[1;33mErreur: Echec d'ecriture de la configuration\033[0m\n",
/*SET_NEW_SETTINGS*/ "\033[1mNouveaux parametres (laissez vide pour defaut)\033[0m\n",
/*SET_SPEED_XY*/ "\033[0mVitesse d'avance rapide des axes X et Y en mm/min (\033[1;33m%d\033[0m): \n > \033[1;32m",
/*SET_SPEED_Z*/ "\033[0mVitesse d'avance rapide de l'axe Z en mm/min (\033[1;33m%d\033[0m): \n > \033[1;32m",
/*SET_SPEED_PERC*/ "\033[0mPoucentage d'avance maximum (\033[1;33m%d\033[0m): \n > \033[1;32m ",
/*SET_GRIDSIZE*/ "\033[0mTaille de la grille maximum en pixels (\033[1;33m%d\033[0m): \n > \033[1;32m ",
/*SET_ARCRES*/ "\033[0mResolution maximum des arcs en pixels (\033[1;33m%d\033[0m): \n > \033[1;32m ",
/*NEW_SETTINGS*/ "\nNouveaux parametres:\nXY: \033[1;32m%dmm/min\033[0m\nZ: \033[1;32m%dmm/min\033[0m\nVitesse: \033[1;32m%d%%\033[0m\nGrille: \033[1;32m%dpx\033[0m\nArc: \033[1;32m%dpx\033[0m\n\n",
/*SAVE_CONFIRM*/ "\033[1;32mEntrer\033[0m to \033[1mSauvegarder\033[0m, \033[1;32mCtrl+C\033[0m to \033[1mQuitter\033[0m"
};

const char **strConfig [] = {strConfigEN, strConfigFR};

//func
bool configSave (bool); //save config file
void configParse (void); //parse/create program config file
void configEdit (bool); //edit config file
void configManu (void); //onfly config edit without save

//vars
extern bool debug; //debug mode bool
extern unsigned int speedFastXY; //max XY moving speed
extern unsigned int speedFastZ;  //max Z moving speed
extern unsigned int speedPercent; //max running percent speed, used as a override
extern unsigned int gdWidth; //max grid width/height in px
extern unsigned int gdArcRes; //arc drawing resolution
extern unsigned int language; //language id
unsigned int *configArr [] = {&speedFastXY, &speedFastZ, &speedPercent, &gdWidth, &gdArcRes};
unsigned int configArrSize = sizeof(configArr) / sizeof(*configArr);

#endif