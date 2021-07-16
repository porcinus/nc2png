/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to multilanguage support.
*/

#include "lang.h"

#if defined _WIN32 || defined __CYGWIN__
    unsigned int getLocale (void) {
        char sysLocale[8];
        int retLen = GetLocaleInfo(GetSystemDefaultUILanguage(), LOCALE_SISO639LANGNAME, sysLocale, sizeof(sysLocale));
        if(debug){fprintf(stderr,"DEBUG: WIN: sysLocale:%s\n", sysLocale);}
        if (retLen > 0) {
            if (strcmp (sysLocale, "fr") == 0) {return LANG_FR;}
        }
        return LANG_EN;
    }
#else
    unsigned int getLocale (void) {
        char *sysLocale = setlocale(LC_ALL, NULL), langIdent [3];
        //strcmp (sysLocale, *(setlocale(LC_ALL, NULL)));
        if(debug){fprintf(stderr,"DEBUG: LINUX: sysLocale:%s\n", sysLocale);}
        if (strlen (sysLocale) > 1) {
            strncpy (langIdent, sysLocale, 2);
            if (strcmp (langIdent, "fr") == 0) {return LANG_FR;}
        }
        return LANG_EN;
    }
#endif

