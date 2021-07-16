/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Windows specific.
*/

#include "win.h"

bool CtrlHandler(DWORD signal) { //handle ctrl-c on windows to avoid ansicon glitch that doesn't reset ansi code
    if (signal == CTRL_C_EVENT) {
        printf ("\033[0m");
        if(debug){fprintf(stderr,"DEBUG: Ctrl+C triggered\n");}
        exit (0);
    }
    return true;
}
