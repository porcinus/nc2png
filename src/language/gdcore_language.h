/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

gdcore.h/.cpp language file

upd 0.4a ok
*/

#ifndef GDCORE_LANGUAGE_H
#define GDCORE_LANGUAGE_H
#include "lang.h"



enum STR_GDCORE {GDCORE_PROGRESS, GDCORE_TOOL, GDCORE_GRID, GDCORE_AXIS, GDCORE_TOOLPATHS, GDCORE_FAST, GDCORE_LINEAR, GDCORE_CIRCULAR, GDCORE_DURATION, GDCORE_FEED, GDCORE_PNGPATH};

const char *strGDCore[][LANG_COUNT] = {
    { /*GDCORE_PROGRESS*/
        "Progress: %d/%d",
        "Progression: %d/%d",
    },
    { /*GDCORE_TOOL*/
        "Tool",
        "Outil",
    },
    { /*GDCORE_GRID*/
        "Grid",
        "Grille",
    },
    { /*GDCORE_AXIS*/
        "Axis",
        "Axes",
    },
    { /*GDCORE_TOOLPATHS*/
        "Toolpaths",
        "Chemin Outils",
    },
    { /*GDCORE_FAST*/
        "Fast",
        "Rapide",
    },
    { /*GDCORE_LINEAR*/
        "Linear Interpolation",
        "Interpolation Lineaire",
    },
    { /*GDCORE_CIRCULAR*/
        "Circular Interpolation",
        "Interpolation Circulaire",
    },
    { /*GDCORE_DURATION*/
        "Theorical duration",
        "Temps Total Theorique",
    },
    { /*GDCORE_FEED*/
        "Feed",
        "Avance",
    },
    { /*GDCORE_PNGPATH*/
        "Preview path",
        "Chemin de l'apercu",
    },
};



#endif