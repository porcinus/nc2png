/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Specific to Linux.

upd 0.4a ok
*/

#include "linux.h"

bool runningFromTerminal(void){ //try to detect if program running from terminal
    //for whatever reason, isatty() and ptsname() can both fails sometime
    if (isatty(fileno(stdin)) || ptsname(fileno(stdin)) != NULL){
        debug_stdout("running from terminal\n");
        return true;
    }

    //try another way to detect terminal
    char buffer[128];
    sprintf(buffer, "ps -ho tty -p %d 2>/dev/null", getpid()); //get tty/pts from ps command
    FILE* processHandle = popen(buffer, "r");
    if (processHandle == NULL){
        debug_stdout("failed to run 'ps' command\n");
        return true; //avoid run loop
    }

    fgets(buffer, 127, processHandle); pclose(processHandle);
    if (strlen(buffer) && buffer[0] != '?'){
        debug_stdout("running from terminal\n");
        return true;
    }

    debug_stdout("failed to detect terminal\n");
    return false;
}

const char* terminalFilePath(void){ //try to get proper terminal executable file
    for (int i = 0; i < terminalArrCount; i++){
        char buffer[strlen(terminalCmd[i])]; strcpy(buffer, terminalCmd[i]); //buffer backup
        char* tmpPtr = strchr(buffer, ' '); if (tmpPtr != NULL){*tmpPtr = '\0';} //cut command line at first space
        struct stat fileStats;
        if (stat(buffer, &fileStats) == 0){
            debug_stdout("%s found\n", buffer);
            return (char*)terminalCmd[i];
        }
    }
    return NULL;
}

