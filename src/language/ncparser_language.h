/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

ncparser.h/.cpp language file

upd 0.4a ok
*/

#ifndef NCPARSER_LANGUAGE_H
#define NCPARSER_LANGUAGE_H
#include "lang.h"


//parser report
enum STR_PARSER_REPORT {
PARSER_REPORT_NC_NOCOMMENT, PARSER_REPORT_TITLE, 
PARSER_REPORT_SPEED_XY, PARSER_REPORT_SPEED_Z, 
PARSER_REPORT_SETTINGS_TITLE, PARSER_REPORT_SETTINGS_COORD_LINE, PARSER_REPORT_SETTINGS_COORD_CIRC, PARSER_REPORT_SETTINGS_UNIT, PARSER_REPORT_SETTINGS_UNIT_NOTSUPP, PARSER_REPORT_SETTINGS_COMP, PARSER_REPORT_SETTINGS_COMP_NOTSUPP, PARSER_REPORT_SETTINGS_WORKPLANE, PARSER_REPORT_SETTINGS_WORKPLANE_NOTSUPP, 
PARSER_REPORT_TIME_TITLE, PARSER_REPORT_TIME_PERCENT_TITLE, PARSER_REPORT_TIME_FAST, PARSER_REPORT_TIME_WORK, PARSER_REPORT_TIME_CIRC, PARSER_REPORT_TIME_DRILL, PARSER_REPORT_TIME_TOTAL, PARSER_REPORT_TIME_COMMENT_TITLE, 
PARSER_REPORT_LIMITS_TITLE, PARSER_REPORT_LIMITS_x, 
PARSER_REPORT_TOOLS_TITLE, 
PARSER_REPORT_TOOLPATHS_TITLE, PARSER_REPORT_TOOLPATHS_FAST, PARSER_REPORT_TOOLPATHS_WORK, PARSER_REPORT_TOOLPATHS_CIRC, PARSER_REPORT_TOOLPATHS_DRILL, PARSER_REPORT_TOOLPATHS_TOTAL, 
PARSER_REPORT_GCODE_TITLE, PARSER_REPORT_GCODE_TOL, PARSER_REPORT_GCODE_COMMENTED, PARSER_REPORT_GCODE_IGNORED, PARSER_REPORT_GCODE_Gx, PARSER_REPORT_GCODE_DRILL, 
PARSER_REPORT_WARNING_TOOLCRASH, 
};

const char *strParserReportTerm[][LANG_COUNT] = {
    { /*PARSER_REPORT_NC_NOCOMMENT*/
        "Program",
        "Programme",
    },
    { /*PARSER_REPORT_TITLE*/
        "Report for '\033[1;33m%s\033[0m' file:\n",
        "Rapport pour le fichier '\033[1;33m%s\033[0m':\n",
    },
//
    { /*PARSER_REPORT_SPEED_XY*/
        "	Max XY speed:     \033[1;33m%u\033[0mmm/min\n",
        "	Vitesse maximum des axes X et Y:     \033[1;33m%u\033[0mmm/min\n",
    },
    { /*PARSER_REPORT_SPEED_Z*/
        "	Max Z speed:      \033[1;33m%u\033[0mmm/min\n",
        "	Vitesse maximum de l'axe Z:          \033[1;33m%u\033[0mmm/min\n",
    },
//
    { /*PARSER_REPORT_SETTINGS_TITLE*/
        "	Parameters:\n",
        "	Paramètres:\n",
    },
    { /*PARSER_REPORT_SETTINGS_COORD_LINE*/
        "		Linear Coordinates:   \033[1;33mG%d\033[0m\n",
        "		Coordonnées linéaire:   \033[1;33mG%d\033[0m\n",
    },
    { /*PARSER_REPORT_SETTINGS_COORD_CIRC*/
        "		Circular Coordinates: \033[1;33mG%d\033[0m\n",
        "		Coordonnées circulaire: \033[1;33mG%d\033[0m\n",
    },
    { /*PARSER_REPORT_SETTINGS_UNIT*/
        "		Units:                \033[1;33mG%d\033[0m",
        "		Unités:                 \033[1;33mG%d\033[0m",
    },
    { /*PARSER_REPORT_SETTINGS_UNIT_NOTSUPP*/
        " \033[1;31m(INCH NOT CURRENTLY SUPPORTED)\033[0m\n",
        " \033[1;31m(POUCE NON SUPPORTÉ)\033[0m\n",
    },
    { /*PARSER_REPORT_SETTINGS_COMP*/
        "		Compensation:         \033[1;33mG%d\033[0m",
        "		Compensation:           \033[1;33mG%d\033[0m",
    },
    { /*PARSER_REPORT_SETTINGS_COMP_NOTSUPP*/
        " \033[1;31m(ONLY G40 CURRENTLY SUPPORTED)\033[0m\n",
        " \033[1;31m(UNIQUEMENT G40 SUPPORTÉ)\033[0m\n",
    },
    { /*PARSER_REPORT_SETTINGS_WORKPLANE*/
        "		Plane:                \033[1;33mG%d\033[0m",
        "		Plan:                   \033[1;33mG%d\033[0m",
    },
    { /*PARSER_REPORT_SETTINGS_WORKPLANE_NOTSUPP*/
        " \033[1;31m(ONLY G17 CURRENTLY SUPPORTED)\033[0m\n",
        " \033[1;31m(UNIQUEMENT G17 SUPPORTÉ)\033[0m\n",
    },
//
    { /*PARSER_REPORT_TIME_TITLE*/
        "Times:\n",
        "Rapport de temps:\n",
    },
    { /*PARSER_REPORT_TIME_PERCENT_TITLE*/
        "Times (\033[1;33m%u%%\033[0m):\n",
        "Rapport de temps (\033[1;33m%u%%\033[0m):\n",
    },
    { /*PARSER_REPORT_TIME_FAST*/
        "	Fast mode:      \033[1;36m%s\033[0m\n",
        "	Temps en avance rapide:              \033[1;36m%s\033[0m\n",
    },
    { /*PARSER_REPORT_TIME_WORK*/
        "	Work mode:      \033[1;36m%s\033[0m\n",
        "	Temps en avance travail:             \033[1;36m%s\033[0m\n",
    },
    { /*PARSER_REPORT_TIME_CIRC*/
        "	Circular mode:  \033[1;36m%s\033[0m\n",
        "	Temps en interpolation circulaire:   \033[1;36m%s\033[0m\n",
    },
    { /*PARSER_REPORT_TIME_DRILL*/
        "	Drilling mode:  \033[1;36m%s\033[0m\n",
        "	Temps en cycle de percage:           \033[1;36m%s\033[0m\n",
    },
    { /*PARSER_REPORT_TIME_TOTAL*/
        "	Total time: \033[1;36m%s\033[0m\n",
        "	Temps Total Théorique:               \033[1;36m%s\033[0m\n",
    },
    { /*PARSER_REPORT_TIME_COMMENT_TITLE, second %s used for padding*/
        "		'\033[1;32m%s\033[0m':%s\033[1;36m%s\033[0m\n",
        "		'\033[1;32m%s\033[0m':%s\033[1;36m%s\033[0m\n",
    },
//
    { /*PARSER_REPORT_LIMITS_TITLE*/
        "Limits:\n",
        "Limites:\n",
    },
    { /*PARSER_REPORT_LIMITS_x*/
        "	Extremes %s: \033[1;36m%.02lfmm\033[0m, \033[1;36m%.02lfmm\033[0m (\033[1;32m%.02lfmm\033[0m)\n",
        "	Extrêmes %s: \033[1;36m%.02lfmm\033[0m, \033[1;36m%.02lfmm\033[0m (\033[1;32m%.02lfmm\033[0m)\n",
    },
//
    { /*PARSER_REPORT_TOOLS_TITLE*/
        "Detected tool(s):\n",
        "Outil(s) détecté:\n",
    },
//
    { /*PARSER_REPORT_TOOLPATHS_TITLE*/
        "Toolpath moves:\n",
        "Rapport de chemin d'outil:\n",
    },
    { /*PARSER_REPORT_TOOLPATHS_FAST*/
        "	Fast mode:      \033[1;36m%.02lfmm\033[0m\n",
        "	Distance en avance rapide:              \033[1;36m%.02lfmm\033[0m\n",
    },
    { /*PARSER_REPORT_TOOLPATHS_WORK*/
        "	Work mode:      \033[1;36m%.02lfmm\033[0m\n",
        "	Distance en avance travail:             \033[1;36m%.02lfmm\033[0m\n",
    },
    { /*PARSER_REPORT_TOOLPATHS_CIRC*/
        "	Circular mode:  \033[1;36m%.02lfmm\033[0m\n",
        "	Distance en interpolation circulaire:   \033[1;36m%.02lfmm\033[0m\n",
    },
    { /*PARSER_REPORT_TOOLPATHS_DRILL*/
        "	Drilling mode:  \033[1;36m%.02lfmm\033[0m\n",
        "	Distance en cycle de percage:           \033[1;36m%.02lfmm\033[0m\n",
    },
    { /*PARSER_REPORT_TOOLPATHS_TOTAL*/
        "	Total distance: \033[1;32m%.02lfmm\033[0m\n",
        "	Distance Total Theorique:               \033[1;32m%.02lfmm\033[0m\n",
    },
//
    { /*PARSER_REPORT_GCODE_TITLE*/
        "Gcode:\n",
        "Rapport relatif au Gcode:\n",
    },
    { /*PARSER_REPORT_GCODE_TOL*/
        "	TOL:                \033[1;36m%d\033[0m\n",
        "	Lignes dans le programme: \033[1;36m%d\033[0m\n",
    },
    { /*PARSER_REPORT_GCODE_COMMENTED*/
        "	Lines with comment: \033[1;36m%d\033[0m\n",
        "	Lignes avec commentaire:  \033[1;36m%d\033[0m\n",
    },
    { /*PARSER_REPORT_GCODE_IGNORED*/
        "	Ignored lines:      \033[1;36m%d\033[0m\n",
        "	Lignes ignorées:          \033[1;36m%d\033[0m\n",
    },
    { /*PARSER_REPORT_GCODE_Gx*/
        "	G%s lines:           \033[1;36m%d\033[0m\n",
        "	Ligne en déplacement G%s:  \033[1;36m%d\033[0m\n",
    },
    { /*PARSER_REPORT_GCODE_DRILL*/
        "	G81-83 drill cycle: \033[1;36m%d\033[0m\n",
        "	Cycle de percage G81-83:  \033[1;36m%d\033[0m\n",
    },
//
    { /*PARSER_REPORT_WARNING_TOOLCRASH*/
        "\033[1;31mWARNING: Possible tool crash:\nZ min (%.2fmm) < Z min work (%.2fmm)\033[0m\n",
        "\033[1;31mATTENTION: possible crash d'outil:\nZ min (%.2fmm) < Z min travail (%.2fmm)\033[0m\n",
    },



};



#endif