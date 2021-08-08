/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to configuration functions/vars.
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <cstdlib>
#include <cstring>

//enum LANG_ID {LANG_EN, LANG_FR} ;
enum STR_CONFIG {RESET_SUCCESS,SAVE_SUCCESS,WRITE_FAILED,SET_NEW_SETTINGS,NEW_SETTINGS,SAVE_CONFIRM};

const char *strConfigEN [] = {
/*RESET_SUCCESS*/ "\033[1;32mConfig file reset with success\033[0m\n",
/*SAVE_SUCCESS*/ "\033[1;32mConfig saved successfully\033[0m\n",
/*WRITE_FAILED*/ "\033[1;33mError: Failed to write config file\033[0m\n",
/*SET_NEW_SETTINGS*/ "\033[1mSet new settings (leave blank to keep unchanged)\033[0m\n",
/*NEW_SETTINGS*/ "\nNew settings:\n",
/*SAVE_CONFIRM*/ "Press \033[1;32mEnter\033[0m to \033[1mSave\033[0m, \033[1;32mCtrl+C\033[0m to \033[1mCancel\033[0m"
};

const char *strConfigVarsEN [] = {
"\033[0mMaximum X and Y fast speed in mm/min (\033[1;33m%d\033[0m): \n > \033[1;32m",
"\033[0mMaximum Z fast speed in mm/min (\033[1;33m%d\033[0m): \n > \033[1;32m",
"\033[0mSpeed override in percent (\033[1;33m%d\033[0m): \n > \033[1;32m ",
"\033[0mPreview grid max width/height in pixels (\033[1;33m%d\033[0m): \n > \033[1;32m ",
"\033[0mPreview arc resolution in pixels (\033[1;33m%d\033[0m): \n > \033[1;32m ",
"\033[0mEnable OpenGL preview, 0:No 1:Yes (\033[1;33m%d\033[0m): \n > \033[1;32m ",
"\033[0mOpenGL: Viewport width in pixels (\033[1;33m%d\033[0m): \n > \033[1;32m ",
"\033[0mOpenGL: Viewport height in pixels (\033[1;33m%d\033[0m): \n > \033[1;32m ",
"\033[0mOpenGL: Arc resolution in mm (\033[1;33m%f\033[0m): \n > \033[1;32m "
};

const char *strConfigVarsOutEN [] = {
"XY: \033[1;32m%dmm/min\033[0m\n",
"Z: \033[1;32m%dmm/min\033[0m\n",
"Speed: \033[1;32m%d%%\033[0m\n",
"Grid: \033[1;32m%dpx\033[0m\n",
"Arc: \033[1;32m%dpx\033[0m\n",
"OpenGL: \033[1;32m%d\033[0m\n",
"Width: \033[1;32m%dpx\033[0m\n",
"Height: \033[1;32m%dpx\033[0m\n",
"Arc: \033[1;32m%fmm\033[0m\n\n"
};

const char *strConfigFR [] = {
/*RESET_SUCCESS*/ "\033[1;32mConfiguration reinitialisee avec succes\033[0m\n",
/*SAVE_SUCCESS*/ "\033[1;32mConfiguration sauvegardee avec succes\033[0m\n",
/*WRITE_FAILED*/ "\033[1;33mErreur: Echec d'ecriture de la configuration\033[0m\n",
/*SET_NEW_SETTINGS*/ "\033[1mNouveaux parametres (laissez vide pour defaut)\033[0m\n",
/*NEW_SETTINGS*/ "\nNouveaux parametres:\n",
/*SAVE_CONFIRM*/ "\033[1;32mEntrer\033[0m to \033[1mSauvegarder\033[0m, \033[1;32mCtrl+C\033[0m to \033[1mQuitter\033[0m"
};

const char *strConfigVarsFR [] = {
"\033[0mVitesse d'avance rapide des axes X et Y en mm/min (\033[1;33m%d\033[0m): \n > \033[1;32m",
"\033[0mVitesse d'avance rapide de l'axe Z en mm/min (\033[1;33m%d\033[0m): \n > \033[1;32m",
"\033[0mPoucentage d'avance maximum (\033[1;33m%d\033[0m): \n > \033[1;32m ",
"\033[0mTaille de la grille maximum en pixels (\033[1;33m%d\033[0m): \n > \033[1;32m ",
"\033[0mResolution maximum des arcs en pixels (\033[1;33m%d\033[0m): \n > \033[1;32m ",
"\033[0mActiver l'apercu OpenGL, 0:Non 1:Oui (\033[1;33m%d\033[0m): \n > \033[1;32m ",
"\033[0mOpenGL: Largeur de la fenetre (\033[1;33m%d\033[0m): \n > \033[1;32m ",
"\033[0mOpenGL: Hauteur de la fenetre (\033[1;33m%d\033[0m): \n > \033[1;32m ",
"\033[0mOpenGL: Resolution maximum des arcs en mm (\033[1;33m%f\033[0m): \n > \033[1;32m "
};

const char *strConfigVarsOutFR [] = {
"XY: \033[1;32m%dmm/min\033[0m\n",
"Z: \033[1;32m%dmm/min\033[0m\n",
"Vitesse: \033[1;32m%d%%\033[0m\n",
"Grille: \033[1;32m%dpx\033[0m\n",
"Arc: \033[1;32m%dpx\033[0m\n",
"OpenGL: \033[1;32m%d\033[0m\n",
"Largeur: \033[1;32m%dpx\033[0m\n",
"Hauteur: \033[1;32m%dpx\033[0m\n",
"Arc: \033[1;32m%fmm\033[0m\n\n"
};

const char **strConfig [] = {strConfigEN, strConfigFR};
const char **strConfigVars [] = {strConfigVarsEN, strConfigVarsFR};
const char **strConfigVarsOut [] = {strConfigVarsOutEN, strConfigVarsOutFR};

//func
int inCharArray (char**, char*, unsigned int); //search in char array, return index if found, -1 if not
bool configSave (bool, bool=false); //save config file
void configParse (void); //parse/create program config file
void configEdit (bool); //edit config file
void configManu (void); //onfly config edit without save

//nc2png.h
extern bool debug; //debug mode bool
extern unsigned int speedFastXY; //max XY moving speed
extern unsigned int speedFastZ;  //max Z moving speed
extern unsigned int speedPercent; //max running percent speed, used as a override
extern unsigned int gdWidth; //max grid width/height in px
extern unsigned int gdArcRes; //arc drawing resolution
extern unsigned int language; //language id

//glcore.h
extern unsigned int glViewportEnable, glViewportWidth, glViewportHeight;
extern float glDrawArcPrecision;

//vars
const char *cfgVarsName[] = {"speedFastXY", "speedFastZ", "speedPercent", "gdWidth", "gdArcRes", "glViewportEnable", "glViewportWidth", "glViewportHeight", "glDrawArcPrecision"}; //config vars names
int cfgVarsType[] = {1, 1, 1, 1, 1, 1, 1, 1, 2}; //types : 0:int, 1:uint, 2:float, 3:double
void *cfgVarsPtr[] = {&speedFastXY, &speedFastZ, &speedPercent, &gdWidth, &gdArcRes, &glViewportEnable, &glViewportWidth, &glViewportHeight, &glDrawArcPrecision}; //pointers to corresponding vars
unsigned int cfgVarsArrSize = sizeof(cfgVarsType) / sizeof(*cfgVarsType); //config pointers array size
int cfgVarsManuArr[] = {0,1,2}; //index of needed inputs at program start








#endif