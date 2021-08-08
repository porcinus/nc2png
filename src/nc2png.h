/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Main file
*/

#ifndef NC2PNG_H
#define NC2PNG_H

char programversion[]="0.3a"; //program version

#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
extern "C" {
#include <png.h>
#include <zlib.h>
#include "gd.h"
}
#if defined _WIN32 || defined __CYGWIN__
    #include <windows.h>
    #include "win.h"
#endif

//terminal enable color var
#if defined __linux__
    bool termColor = true;
#elif defined __APPLE__
    bool termColor = true;
#else
    bool termColor = false;
#endif


enum STR_MAIN {
USAGE_EXAMPLE,USAGE_OPTIONAL,USAGE_NEWCFG,USAGE_DELCFG,USAGE_DEFAULTREG,USAGE_DEBUG,USAGE_LIBRARIES,
ANSICON_MODULE,ANSICON_NEEDED,
NC_NOCOMMENT,
REPORT_L01,REPORT_L02,REPORT_L03,REPORT_L04,REPORT_L05,REPORT_L06,REPORT_L07,REPORT_L08,REPORT_L09,REPORT_L10,
REPORT_L11,REPORT_L12,REPORT_L13,REPORT_L14,REPORT_L15,REPORT_L16,REPORT_L17A,REPORT_L17B,REPORT_L18,REPORT_L19,REPORT_L20,
REPORT_L21,REPORT_L22A,REPORT_L22B,REPORT_L23,REPORT_L24,REPORT_L25,REPORT_L26,REPORT_L27,REPORT_L28,REPORT_L29,REPORT_L30,
REPORT_L31,REPORT_L32,REPORT_L33,REPORT_L34,REPORT_L35,REPORT_L36,REPORT_L37,REPORT_L38,REPORT_L39,REPORT_L40,
REPORT_L41,
GENTIME_NC,GENTIME_GD,
};

const char *strMainEN [] = {
/*USAGE_EXAMPLE*/ "Example",
/*USAGE_OPTIONAL*/  "Optional",
/*USAGE_NEWCFG*/  "Set new configuration settings",
/*USAGE_DELCFG*/  "Reset current settings",
/*USAGE_DEFAULTREG*/  "Set as default program for .nc files [Windows]",
/*USAGE_DEBUG*/  "Enable debug",
/*USAGE_LIBRARIES*/ "External libraries",

/*ANSICON_MODULE*/ "This program require ANSICON to be installed and its module running",
/*ANSICON_NEEDED*/ "This program needs a ANSI compatible terminal to run",

/*NC_NOCOMMENT*/ "Program",

/*REPORT_L01*/ "Rapport pour le fichier '\033[1;33m%s\033[0m':\n",
/*REPORT_L02*/ "	Max XY speed :     \033[1;33m%d\033[0mmm/min\n",
/*REPORT_L03*/ "	Max Z speed  :     \033[1;33m%d\033[0mmm/min\n",
/*REPORT_L04*/ "	Parameters :\n",
/*REPORT_L05*/ "		Lineaire Coordinates : \033[1;33mG%d\033[0m\n",
/*REPORT_L06*/ "		Circular Coordinates : \033[1;33mG%d\033[0m\n",
/*REPORT_L07*/ "		Units :                \033[1;33mG%d\033[0m",
/*REPORT_L08*/ " \033[1;31m(INCH NOT CURRENTLY SUPPORTED)\033[0m\n",
/*REPORT_L09*/ "		Compensation :         \033[1;33mG%d\033[0m",
/*REPORT_L10*/ " \033[1;31m(ONLY G40 CURRENTLY SUPPORTED)\033[0m\n",
/*REPORT_L11*/ "		Plane :                \033[1;33mG%d\033[0m",
/*REPORT_L12*/ " \033[1;31m(ONLY G17 CURRENTLY SUPPORTED)\033[0m\n",
/*REPORT_L13*/ "Times:\n",
/*REPORT_L14*/ "	Work mode :  \033[1;36m%s\033[0m\n",
/*REPORT_L15*/ "	Fast mode :  \033[1;36m%s\033[0m\n",
/*REPORT_L16*/ "	Total time : \033[1;36m%s\033[0m\n",
/*REPORT_L17A*/"		'\033[1;32m%s\033[0m'\n",
/*REPORT_L17B*/"		Work : \033[1;36m%s\033[0m - Fast : \033[1;36m%s\033[0m\n",
/*REPORT_L18*/ "Times (\033[1;33m%d%%\033[0m):\n",
/*REPORT_L19*/ "	Work mode :  \033[1;36m%s\033[0m\n",
/*REPORT_L20*/ "	Fast mode :  \033[1;36m%s\033[0m\n",
/*REPORT_L21*/ "	Total time : \033[1;36m%s\033[0m\n",
/*REPORT_L22A*/"		'\033[1;32m%s\033[0m'\n",
/*REPORT_L22B*/"		Work : \033[1;36m%s\033[0m - Fast : \033[1;36m%s\033[0m\n",
/*REPORT_L23*/ "Limits:\n",
/*REPORT_L24*/ "	Extremes X : \033[1;36m%.02lfmm\033[0m , \033[1;36m%.02lfmm\033[0m (\033[1;32m%.02lfmm\033[0m)\n",
/*REPORT_L25*/ "	Extremes Y : \033[1;36m%.02lfmm\033[0m , \033[1;36m%.02lfmm\033[0m (\033[1;32m%.02lfmm\033[0m)\n",
/*REPORT_L26*/ "	Extremes Z : \033[1;36m%.02lfmm\033[0m , \033[1;36m%.02lfmm\033[0m (\033[1;32m%.02lfmm\033[0m)\n",
/*REPORT_L27*/ "Toolpath moves:\n",
/*REPORT_L28*/ "	Work mode :      \033[1;36m%.02lfmm\033[0m\n",
/*REPORT_L29*/ "	Fast mode :      \033[1;36m%.02lfmm\033[0m\n",
/*REPORT_L30*/ "	Circular mode :  \033[1;36m%.02lfmm\033[0m\n",
/*REPORT_L31*/ "	Drilling mode :  \033[1;36m%.02lfmm\033[0m\n",
/*REPORT_L32*/ "	Total distance : \033[1;32m%.02lfmm\033[0m\n",
/*REPORT_L33*/ "Gcode:\n",
/*REPORT_L34*/ "	TOL :                \033[1;36m%d\033[0m\n",
/*REPORT_L35*/ "	Comment lines :      \033[1;36m%d\033[0m\n",
/*REPORT_L36*/ "	Ignored lines :      \033[1;36m%d\033[0m\n",
/*REPORT_L37*/ "	G0 lines :           \033[1;36m%d\033[0m\n",
/*REPORT_L38*/ "	G1 lines :           \033[1;36m%d\033[0m\n",
/*REPORT_L39*/ "	G2 lines :           \033[1;36m%d\033[0m\n",
/*REPORT_L40*/ "	G3 lines :           \033[1;36m%d\033[0m\n",
/*REPORT_L41*/ "	G81-83 drill cycle : \033[1;36m%d\033[0m\n",
            
/*GENTIME_NC*/ "Gcode parsing took \033[1;36m%.3lfsec\033[0m\n",
/*GENTIME_GD*/ "Preview generation took \033[1;36m%.3lfsec\033[0m\n"
};

const char *strMainFR [] = {
/*USAGE_EXAMPLE*/ "Exemple",
/*USAGE_OPTIONAL*/  "Optionel",
/*USAGE_NEWCFG*/  "Definir une nouvelle configuration",
/*USAGE_DELCFG*/  "Supprimer la configuration",
/*USAGE_DEFAULTREG*/  "Definir comme programme par defaut pour les fichiers .nc [Windows]",
/*USAGE_DEBUG*/  "Activer le debuggage",
/*USAGE_LIBRARIES*/ "External libraries",

/*ANSICON_MODULE*/ "Ce programme necessite l'installation de ANSICON et son module soit fonctionnel",
/*ANSICON_NEEDED*/ "Ce programme necessite un terminal compatible ANSI",

/*NC_NOCOMMENT*/ "Programme",

/*REPORT_L01*/ "Rapport pour le fichier '\033[1;33m%s\033[0m':\n",
/*REPORT_L02*/ "	Vitesse maximum des axes X et Y :     \033[1;33m%d\033[0mmm/min\n",
/*REPORT_L03*/ "	Vitesse maximum de l'axe Z :          \033[1;33m%d\033[0mmm/min\n",
/*REPORT_L04*/ "	Parametres :\n",
/*REPORT_L05*/ "		Coordonnees lineaire :   \033[1;33mG%d\033[0m\n",
/*REPORT_L06*/ "		Coordonnees circulaire : \033[1;33mG%d\033[0m\n",
/*REPORT_L07*/ "		Unites :                 \033[1;33mG%d\033[0m",
/*REPORT_L08*/ " \033[1;31m(POUCE NON SUPPORTE)\033[0m\n",
/*REPORT_L09*/ "		Compensation :           \033[1;33mG%d\033[0m",
/*REPORT_L10*/ " \033[1;31m(UNIQUEMENT G40 SUPPORTE)\033[0m\n",
/*REPORT_L11*/ "		Plan :                   \033[1;33mG%d\033[0m",
/*REPORT_L12*/ " \033[1;31m(UNIQUEMENT G17 SUPPORTE)\033[0m\n",
/*REPORT_L13*/ "Rapport de temps:\n",
/*REPORT_L14*/ "	Temps en avance travail : \033[1;36m%s\033[0m\n",
/*REPORT_L15*/ "	Temps en avance rapide :  \033[1;36m%s\033[0m\n",
/*REPORT_L16*/ "	Temps Total Theorique :   \033[1;36m%s\033[0m\n",
/*REPORT_L17A*/"		'\033[1;32m%s\033[0m'\n",
/*REPORT_L17B*/"		Travail : \033[1;36m%s\033[0m - Rapide : \033[1;36m%s\033[0m\n",
/*REPORT_L18*/ "Rapport de temps (\033[1;33m%d%%\033[0m):\n",
/*REPORT_L19*/ "	Temps en avance travail : \033[1;36m%s\033[0m\n",
/*REPORT_L20*/ "	Temps en avance rapide :  \033[1;36m%s\033[0m\n",
/*REPORT_L21*/ "	Temps Total Theorique :   \033[1;36m%s\033[0m\n",
/*REPORT_L22A*/"		'\033[1;32m%s\033[0m'\n",
/*REPORT_L22B*/"		Travail : \033[1;36m%s\033[0m - Rapide : \033[1;36m%s\033[0m\n",
/*REPORT_L23*/ "Limites:\n",
/*REPORT_L24*/ "	Extremes X : \033[1;36m%.02lfmm\033[0m , \033[1;36m%.02lfmm\033[0m (\033[1;32m%.02lfmm\033[0m)\n",
/*REPORT_L25*/ "	Extremes Y : \033[1;36m%.02lfmm\033[0m , \033[1;36m%.02lfmm\033[0m (\033[1;32m%.02lfmm\033[0m)\n",
/*REPORT_L26*/ "	Extremes Z : \033[1;36m%.02lfmm\033[0m , \033[1;36m%.02lfmm\033[0m (\033[1;32m%.02lfmm\033[0m)\n",
/*REPORT_L27*/ "Rapport de chemin d'outil:\n",
/*REPORT_L28*/ "	Distance en avance travail :                              \033[1;36m%.02lfmm\033[0m\n",
/*REPORT_L29*/ "	Distance en avance rapide :                               \033[1;36m%.02lfmm\033[0m\n",
/*REPORT_L30*/ "	Distance en avance travail par interpolation circulaire : \033[1;36m%.02lfmm\033[0m\n",
/*REPORT_L31*/ "	Distance en cycle de percage :                            \033[1;36m%.02lfmm\033[0m\n",
/*REPORT_L32*/ "	Distance Total Theorique :                                \033[1;32m%.02lfmm\033[0m\n",
/*REPORT_L33*/ "Rapport relatif au Gcode:\n",
/*REPORT_L34*/ "	Lignes dans le programme : \033[1;36m%d\033[0m\n",
/*REPORT_L35*/ "	Lignes de commentaire :    \033[1;36m%d\033[0m\n",
/*REPORT_L36*/ "	Lignes ignorees :          \033[1;36m%d\033[0m\n",
/*REPORT_L37*/ "	Ligne en deplacement G0 :  \033[1;36m%d\033[0m\n",
/*REPORT_L38*/ "	Ligne en deplacement G1 :  \033[1;36m%d\033[0m\n",
/*REPORT_L39*/ "	Ligne en deplacement G2 :  \033[1;36m%d\033[0m\n",
/*REPORT_L40*/ "	Ligne en deplacement G3 :  \033[1;36m%d\033[0m\n",
/*REPORT_L41*/ "	Cycle de percage G81 G83 : \033[1;36m%d\033[0m\n",
            
/*GENTIME_NC*/ "Temps de traitement du code NC : \033[1;36m%.3lfsec\033[0m\n",
/*GENTIME_GD*/ "Temps de generation de l'apecu : \033[1;36m%.3lfsec\033[0m\n"
};

const char **strMain [] = {strMainEN, strMainFR};


//ansicon.h
extern int checkAnsiconExists (void); //search thru PATH for ansicon
extern int checkAnsiconModule (void); //search thru current process for ansicon module loaded

//config.h
extern bool configSave (bool, bool=false); //save config file
extern void configParse (void); //parse/create program config file
extern void configEdit (bool); //edit config file
extern void configManu (void); //onfly config edit without save

//gdcore.h
extern void sec2charArr (char*, double); //convert seconds to char array in format : XXsec / XXmin / XXh XXmin
extern int gdPreview (char*, int, int, /*ncFlagsStruc*, */ncLineStruc*, ncToolStruc*, ncDistTimeStruc*, ncLimitStruc*, ncLinesCountStruc*, bool); //generate image preview off ncparser data

//glcore.h
extern unsigned int glViewportEnable;
extern int glPreview (ncLineStruc*, ncToolStruc*, ncDistTimeStruc*, ncLimitStruc*, ncLinesCountStruc*, ncArraySize*, bool);

//lang.h
extern unsigned int getLocale (void); //get system locale id

//vars
bool debug = false; //debug mode bool
unsigned int speedFastXY = 2500; //max XY moving speed
unsigned int speedFastZ = 1000;  //max Z moving speed
unsigned int speedPercent = 100; //max running percent speed, used as a override
unsigned int gdWidth = 800; //max grid width/height in px
unsigned int gdArcRes = 10; //arc drawing resolution
unsigned int ncCommentsLimit = 100; //comments array limit
unsigned int ncToolsLimit = 100; //tools array limit
bool shouldRunAnsicon = false; //used to avoid run loop if ansicon not installed in windows, NO NOT EDIT
unsigned int language = 0; //language id
char libGDver[] = GD_VERSION_STRING;
char libPNGver[] = PNG_LIBPNG_VER_STRING;
char zlibver[] = ZLIB_VERSION;



#endif