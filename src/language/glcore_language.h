/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

glcore.h/.cpp language file

upd 0.4a ok
*/

#ifndef GLCORE_LANGUAGE_H
#define GLCORE_LANGUAGE_H
#include "lang.h"

enum STR_GLCORE {
GLCORE_PROGRESS,
GLCORE_NC_NOCOMMENT, 
GLCORE_LAYER_DISPLAY, GLCORE_LAYER_HIDE, GLCORE_LAYER_REPORT, GLCORE_LAYER_LAYERS, 
GLCORE_TOOL_FAST, GLCORE_TOOL_WORK, GLCORE_TOOL_CIRC, GLCORE_TOOL_DRILL, GLCORE_TOOL_TOTAL, GLCORE_TOOL_ZCLIPPING, 
GLCORE_MENU_TOOLPATH, GLCORE_MENU_DISPLAY, GLCORE_MENU_RESETVIEW, GLCORE_MENU_AXIS, GLCORE_MENU_GRID, GLCORE_MENU_SUBGRID, GLCORE_MENU_LIMITS, GLCORE_MENU_HEIGHTMAP, GLCORE_MENU_EXPORT, GLCORE_MENU_SETTINGS, GLCORE_MENU_WIREFRAME, GLCORE_MENU_AA, GLCORE_MENU_LIMITFPS, GLCORE_MENU_FORCEUPD, GLCORE_MENU_CFG, GLCORE_MENU_ABOUT, 
GLCORE_EXPORT_WINDOW_TITLE, GLCORE_EXPORT_VERTICES_COUNT, GLCORE_EXPORT_TRIANGLES_COUNT, GLCORE_EXPORT_OBJ, GLCORE_EXPORT_STL, GLCORE_EXPORT_TIME_REPORT, 
GLCORE_CFG_APPLYWARN, 
GLCORE_WINDOW_SAVE, GLCORE_WINDOW_RESETDEFAULT, GLCORE_WINDOW_CANCEL, GLCORE_ABOUT_EXTLIB, GLCORE_WINDOW_CLOSE, 
};

const char *strGLCore[][LANG_COUNT] = {
    { /*GLCORE_PROGRESS*/
        "Progress: %d/%d",
        "Progression: %d/%d",
    },
//
    { /*GLCORE_NC_NOCOMMENT*/
        "Program",
        "Programme",
    },
//
    { /*GLCORE_LAYER_DISPLAY*/
        "Show",
        "Afficher",
    },
    { /*GLCORE_LAYER_HIDE*/
        "Hide",
        "Masquer",
    },
    { /*GLCORE_LAYER_REPORT*/
        "Report",
        "Rapport",
    },
    { /*GLCORE_LAYER_LAYERS*/
        "G-code",
        "G-code",
    },
//
    { /*GLCORE_TOOL_FAST*/
        "Fast",
        "Rapide",
    },
    { /*GLCORE_TOOL_WORK*/
        "Work",
        "Travail",
    },
    { /*GLCORE_TOOL_CIRC*/
        "Circular",
        "Circulaire",
    },
    { /*GLCORE_TOOL_DRILL*/
        "Drilling",
        "Perçage",
    },
    { /*GLCORE_TOOL_TOTAL*/
        "Total",
        "Total",
    },
    { /*GLCORE_TOOL_ZCLIPPING*/
        "Z clipping",
        "Limite Z",
    },
//
    { /*GLCORE_MENU_TOOLPATH*/
        "Toolpaths",
        "Chemin d'outils",
    },
    { /*GLCORE_MENU_DISPLAY*/
        "Display",
        "Affichage",
    },
    { /*GLCORE_MENU_RESETVIEW*/
        "Reset viewport",
        "RàZ de la vue",
    },
    { /*GLCORE_MENU_AXIS*/
        "Axis",
        "Axes",
    },
    { /*GLCORE_MENU_GRID*/
        "Grid",
        "Grille",
    },
    { /*GLCORE_MENU_SUBGRID*/
        "Sub-grid",
        "Sous-grille",
    },
    { /*GLCORE_MENU_LIMITS*/
        "Limits",
        "Limites",
    },
    { /*GLCORE_MENU_HEIGHTMAP*/
        "Heightmap",
        "Heightmap",
    },
    { /*GLCORE_MENU_EXPORT*/
        "Export",
        "Exporter",
    },
    { /*GLCORE_MENU_SETTINGS*/
        "Settings",
        "Paramètres",
    },
    { /*GLCORE_MENU_WIREFRAME*/
        "Wireframe",
        "Wireframe",
    },
    { /*GLCORE_MENU_AA*/
        "Anti-aliasing",
        "Anti-aliasing",
    },
    { /*GLCORE_MENU_LIMITFPS*/
        "Limit to 60fps",
        "Limiter à 60fps",
    },
    { /*GLCORE_MENU_FORCEUPD*/
        "Force viewport update",
        "Forcer la mise à jour",
    },
    { /*GLCORE_MENU_CFG*/
        "Edit configuration",
        "Editer la config",
    },
    { /*GLCORE_MENU_ABOUT*/
        "About...",
        "A propos de...",
    },

//
    { /*GLCORE_EXPORT_WINDOW_TITLE*/
        "Export Heightmap",
        "Exporter la heightmap",
    },
    { /*GLCORE_EXPORT_VERTICES_COUNT*/
        "Vertices",
        "Vertices",
    },
    { /*GLCORE_EXPORT_TRIANGLES_COUNT*/
        "Triangles",
        "Triangles",
    },
    { /*GLCORE_EXPORT_OBJ*/
        "OBJ format",
        "Format OBJ",
    },
    { /*GLCORE_EXPORT_STL*/
        "STL(bin) format",
        "format STL(bin)",
    },
    { /*GLCORE_EXPORT_TIME_REPORT*/
        "File \"%s\" exported in %.5lfsec\n",
        "\"%s\" exporté en %.5lfsec\n",
    },







//
    { /*GLCORE_CFG_APPLYWARN*/
        "Apply after program restart",
        "Appliqué après redémarrage du programme",
    },
//
    { /*GLCORE_WINDOW_SAVE*/
        "Save",
        "Sauvegarder",
    },
    { /*GLCORE_WINDOW_RESETDEFAULT*/
        "Reset config to default",
        "RàZ config",
    },
    { /*GLCORE_WINDOW_CANCEL*/
        "Cancel",
        "Annuler",
    },
    { /*GLCORE_ABOUT_EXTLIB*/
        "External libraries",
        "Librairies externe",
    },
    { /*GLCORE_WINDOW_CLOSE*/
        "Close",
        "Fermer",
    },
};




#endif