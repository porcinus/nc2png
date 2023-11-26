/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

nc2png.h/.cpp language file
*/

#ifndef NC2PNG_LANGUAGE_H
#define NC2PNG_LANGUAGE_H
#include "lang.h"


//generic
enum STR_GENERIC {GENERIC_PRESSKEYCLOSE, GENERIC_PRESSKEYCONTINUE, GENERIC_FILENOTFOUND, GENERIC_FILEISEMPTY, GENERIC_MEMALLOCFAILED};

const char *strGeneric[][LANG_COUNT] = {
    { /*GENERIC_PRESSKEYCLOSE*/
        "Press \033[1;33m[Enter]\033[0m or \033[1;33m[Ctrl]\033[0m+\033[1;33m[C]\033[0m to close",
        "Appuyer sur \033[1;33m[Entrée]\033[0m or \033[1;33m[Ctrl]\033[0m+\033[1;33m[C]\033[0m pour fermer...",
    },
    { /*GENERIC_PRESSKEYCONTINUE*/
        "Press \033[1;33m[Enter]\033[0m to continue",
        "Appuyer sur \033[1;33m[Entrée]\033[0m pour continuer...",
    },
    { /*GENERIC_FILENOTFOUND*/
        "Error: '%s' not found",
        "Erreur: '%s' non trouvé",
    },
    { /*GENERIC_FILEISEMPTY*/
        "Error: '%s' file is empty",
        "Erreur: le fichier '%s' est vide",
    },
    { /*GENERIC_MEMALLOCFAILED*/
        "Error: failed to allocate required memory",
        "Erreur: échec lors de l'allocation de la memoire",
    },
};


//usage
enum STR_USAGE {USAGE_EXAMPLE, USAGE_OPTIONAL, USAGE_CFGNEW, USAGE_CFGEDIT, USAGE_DEFAULTREG, USAGE_DEBUG, USAGE_LIBRARIES};

const char *strUsageTerm[][LANG_COUNT] = {
    { /*USAGE_EXAMPLE*/
        "Example",
        "Exemple",
    },
    { /*USAGE_OPTIONAL*/
        "Optional",
        "Optionnel",
    },
    { /*USAGE_CFGNEW*/
        "Set new configuration settings",
        "Définir une nouvelle configuration",
    },
    { /*USAGE_CFGEDIT*/
        "Edit a configuration variable, format: VAR=VAL",
        "Éditer une variable de configuration, format: VAR=VAL",
    },
    { /*USAGE_DEFAULTREG*/
        "Set as default program for .nc files [Windows]",
        "Définir comme programme par défaut pour les fichiers .nc [Windows]",
    },
    { /*USAGE_DEBUG*/
        "Force enabled debug mode",
        "Forcer le mode debuggage",
    },
    { /*USAGE_LIBRARIES*/
        "External libraries",
        "Librairies externe",
    },
};


//ansicon/ansi specific
enum STR_ANSI {ANSICON_MODULE,ANSI_REQUIRED};

const char *strAnsi[][LANG_COUNT] = {
    { /*ANSICON_MODULE*/
        "This program require ANSICON to be installed and its module running",
        "Ce programme nécessite l'installation de ANSICON et son module soit fonctionnel",
    },
    { /*ANSI_REQUIRED*/
        "This program needs a ANSI compatible terminal to run",
        "Ce programme nécessite un terminal compatible ANSI",
    },
};


//config file related, needs to follow cfg_vars[] order
enum STR_CONFIG_ITEM_TERM {
    DEBUG_TERM, DEBUG_GCODE_TERM, DEBUG_GD_TERM, DEBUG_SVG_TERM, DEBUG_OPENGL_TERM, 
    SPEED_XY_TERM, SPEED_Z_TERM, SPEED_PERCENT_TERM, SKIP_CNC_PROMPT, 
    GD_EN_TERM, GD_WIDTH_TERM, GD_ARCRES_TERM, GD_EXPORTDEPTHMAP_TERM, 
    SVG_EN_TERM, SVG_ARCRES_TERM, 
    GL_EN_TERM, GL_WIDTH_TERM, GL_HEIGHT_TERM, GL_ARCRES_TERM, GL_HEIGHTMAPRES_TERM, 
};

const char *strConfigItem[][LANG_COUNT] = {
    { /*DEBUG_TERM*/
        "Enable debug output, 0:No 1:Yes. Warning, massive CPU overhead.",
        "Activer le debuggage, 0:Non 1:Oui. Attention, grosse augmentation de charge CPU.",
    },
    { /*DEBUG_GCODE_TERM*/
        "Create GCODE debug file, 0:No 1:Yes. Warning, massive overhead (CPU/Storage).",
        "Créer un fichier debuggage GCODE, 0:Non 1:Oui. Attention, grosse augmentation de charge (CPU/Stockage).",
    },
    { /*DEBUG_GD_TERM*/
        "Create PNG debug file, 0:No 1:Yes. Warning, massive overhead (CPU/Storage) if gdProperDepthMap=1",
        "Créer un fichier debuggage PNG, 0:Non 1:Oui. Attention, grosse augmentation de charge (CPU/Stockage) si gdExportDepthMap=1",
    },
    { /*DEBUG_SVG_TERM*/
        "Create SVG debug file, 0:No 1:Yes. Warning, massive overhead (CPU/Storage).",
        "Créer un fichier debuggage SVG, 0:Non 1:Oui. Attention, grosse augmentation de charge (CPU/Stockage).",
    },
    { /*DEBUG_OPENGL_TERM*/
        "Create OPENGL debug file, 0:No 1:Yes. Warning, massive overhead (CPU/Storage).",
        "Créer un fichier debuggage OPENGL, 0:Non 1:Oui. Attention, grosse augmentation de charge (CPU/Stockage).",
    },
//
    { /*SPEED_XY_TERM*/
        "Maximum X and Y fast speed in mm/min",
        "Vitesse d'avance rapide des axes X et Y en mm/min",
    },
    { /*SPEED_Z_TERM*/
        "Maximum Z fast speed in mm/min",
        "Vitesse d'avance rapide de l'axe Z en mm/min",
    },
    { /*SPEED_PERCENT_TERM*/
        "Speed override in percent",
        "Poucentage d'avance maximum",
    },
    { /*SKIP_CNC_PROMPT*/
        "Skip CNC settings prompt before preview",
        "Passer les paramètres CN avant aperçu",
    },
//
    { /*GD_EN_TERM*/
        "PNG: Save output file, 0:No 1:Yes",
        "PNG: Sauvegarder le fichier, 0:Non 1:Oui",
    },
    { /*GD_WIDTH_TERM*/
        "PNG: Grid max width/height in pixels",
        "PNG: Taille de la grille maximum en pixels",
    },
    { /*GD_ARCRES_TERM*/
        "PNG: Arc resolution in pixels",
        "PNG: Résolution maximum des arcs en pixels",
    },
    { /*GD_EXPORTDEPTHMAP_TERM*/
        "PNG: If CutViewer tool(s) detected, export a .png depth map (bigger load on CPU/RAM)",
        "PNG: Si outil(s) CutViewer détecté, exportez une depth map en .png (charge plus importante pour le CPU/RAM)",
    },
//
    { /*SVG_EN_TERM*/
        "SVG: Save output file, 0:No 1:Yes",
        "SVG: Sauvegarder le fichier, 0:Non 1:Oui",
    },
    { /*SVG_ARCRES_TERM*/
        "SVG: Arc resolution",
        "SVG: Résolution maximum des arcs",
    },
//
    { /*GL_EN_TERM*/
        "OpenGL: Enable viewport window, 0:No 1:Yes",
        "OpenGL: Activer la fenetre, 0:Non 1:Oui",
    },
    { /*GL_WIDTH_TERM*/
        "OpenGL: Viewport width in pixels",
        "OpenGL: Largeur de la fenetre",
    },
    { /*GL_HEIGHT_TERM*/
        "OpenGL: Viewport height in pixels",
        "OpenGL: Hauteur de la fenetre",
    },
    { /*GL_ARCRES_TERM*/
        "OpenGL: Arc resolution",
        "OpenGL: Résolution maximum des arcs",
    },
    { /*GL_HEIGHTMAPRES_TERM*/
        "OpenGL: Height map resolution (based on CutViewer data), 1px per given unit",
        "OpenGL: Résolution de la height map (basée sur les données CutViewer), 1px par unitée",
    },
};

//new config creation
enum STR_CONFIG_EDIT_TERM {TERM_NEW_SETTINGS, TERM_SAVE_CONFIRM, TERM_INVALID_EDIT_FORMAT};

const char *strConfigEditTerm[][LANG_COUNT] = {
    { /*TERM_NEW_SETTINGS*/
        "New settings:",
        "Nouveau paramètres:",
    },
    { /*TERM_SAVE_CONFIRM*/
        "Press \033[1;32mEnter\033[0m to \033[1mSave\033[0m, \033[1;32mCtrl+C\033[0m to \033[1mCancel\033[0m",
        "\033[1;32mEntrée\033[0m pour \033[1mSauvegarder\033[0m, \033[1;32mCtrl+C\033[0m pour \033[1mQuitter\033[0m",
    },
    { /*TERM_INVALID_EDIT_FORMAT*/
        "Invalid format, needs to be VAR=VALUE",
        "Format invalide, doit être au format VAR=VALUE",
    },
};


//parser
enum STR_FAILED_TERM {TERM_PARSER_FAILED, TERM_GD_FAILED, TERM_SVG_FAILED, TERM_OPENGL_FAILED};

const char *strParserTerm[][LANG_COUNT] = {
    { /*TERM_PARSER_FAILED*/
        "\033[1;31mGcode parser failed with code %d\033[0m",
        "\033[1;31mLe parser Gcode a échoué avec le code %d\033[0m",
    },
    { /*TERM_GD_FAILED*/
        "\033[1;31mPNG file output failed with code %d\033[0m",
        "\033[1;31mLa generation du fichier PNG a échoué avec le code %d\033[0m",
    },
    { /*TERM_SVG_FAILED*/
        "\033[1;31mSVG file output failed with code %d\033[0m",
        "\033[1;31mLa generation du fichier SVG a échoué avec le code %d\033[0m",
    },
    { /*TERM_OPENGL_FAILED*/
        "\033[1;31mOpenGL failed with code %d\033[0m",
        "\033[1;31mOpenGL a échoué avec le code %d\033[0m",
    },
};


//bench report
enum STR_BENCHMARK_REPORT {BENCHMARK_PARSER, BENCHMARK_GD, BENCHMARK_SVG, BENCHMARK_OPENGL};

const char *strBenchmarkReportTerm[][LANG_COUNT] = {
    { /*BENCHMARK_PARSER*/
        "Gcode parsing took \033[1;36m%.3lfsec\033[0m\n",
        "Temps de traitement du code NC : \033[1;36m%.3lfsec\033[0m\n",
    },
    { /*BENCHMARK_GD*/
        "PNG generation took \033[1;36m%.3lfsec\033[0m\n",
        "Temps de génération de l'aperçu PNG : \033[1;36m%.3lfsec\033[0m\n",
    },
    { /*BENCHMARK_SVG*/
        "SVG generation took \033[1;36m%.3lfsec\033[0m\n",
        "Temps de génération de l'aperçu SVG : \033[1;36m%.3lfsec\033[0m\n",
    },
    { /*BENCHMARK_OPENGL*/
        "OpenGL generation took \033[1;36m%.3lfsec\033[0m\n",
        "Temps de génération de l'aperçu OpenGL : \033[1;36m%.3lfsec\033[0m\n",
    },
};






















#endif