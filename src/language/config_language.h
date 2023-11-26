/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

config.h/.cpp language file

upd 0.4a ok
*/

#ifndef CONFIG_LANGUAGE_H
#define CONFIG_LANGUAGE_H
#include "lang.h"

//terminal
enum STR_CONFIG_TERM {TERM_RESET_SUCCESS, TERM_SAVE_SUCCESS, TERM_WRITE_FAILED, TERM_VAR_UPD, TERM_VAR_UPD_FAILED, TERM_VAR_UPD_MISSING};

const char *strConfigTerm[][LANG_COUNT] = {
    { /*TERM_RESET_SUCCESS*/
        "\033[1;32mConfig file reset with success\033[0m",
        "\033[1;32mConfiguration réinitialisée avec succès\033[0m",
    },
    { /*TERM_SAVE_SUCCESS*/
        "\033[1;32mConfig saved successfully\033[0m",
        "\033[1;32mConfiguration sauvegardée avec succès\033[0m",
    },
    { /*TERM_WRITE_FAILED*/
        "\033[1;33mError: Failed to write config file '%s' (errno:%d)\033[0m",
        "\033[1;33mErreur: Échec d'écriture du fichier de config '%s' (errno:%d)\033[0m",
    },
    { /*TERM_VAR_UPD*/
        "'%s' updated with value '%s'",
        "'%s' assignée avec la valeur '%s'",
    },
    { /*TERM_VAR_UPD_FAILED*/
        "Error: Failed to set '%s' with value '%s'",
        "Erreur: Échec d'assignation de '%s' avec la valeur '%s'",
    },
    { /*TERM_VAR_UPD_MISSING*/
        "Error: variable '%s' not in config set",
        "Erreur: variable '%s' absente de la configuration",
    },
};


#endif