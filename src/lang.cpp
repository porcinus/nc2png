/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to multilanguage support.

upd 0.4a ok
*/

#include "lang.h"

#if defined _WIN32 || defined __CYGWIN__
    unsigned int getLocale(void){ //get system locale id (windows)
        char sysLocale[8] = {0};
        int retLen = GetLocaleInfo(GetSystemDefaultUILanguage(), LOCALE_SISO639LANGNAME, sysLocale, sizeof(sysLocale));
        debug_stderr("WIN: sysLocale:%s\n", sysLocale);
        if (retLen > 0){
            if (strcmp(sysLocale, "fr") == 0){return LANG_FR;}
        }
        return LANG_EN;
    }
#else
    unsigned int getLocale(void){ //get system locale id (linux)
        char *sysLocale = setlocale(LC_ALL, NULL), langIdent[3] = {0};
        debug_stderr("LINUX: sysLocale:%s\n", sysLocale);
        if (strlen(sysLocale) > 1){
            strncpy(langIdent, sysLocale, 2);
            if (strcmp(langIdent, "fr") == 0){return LANG_FR;}
        }
        return LANG_EN;
    }
#endif
