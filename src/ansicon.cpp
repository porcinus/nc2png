/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to Ansicon, used on Windows to provide console ansi color compatibility.
*/

#include "ansicon.h"

#if (!defined WINCON && (defined _WIN32 || defined __CYGWIN__))
int checkAnsiconExists (void) { //search thru PATH for ansicon
    if(debug){fprintf(stderr,"DEBUG: Start search for ansicon.exe in PATH variable\n");}
    struct stat filestat;
    char *envPathStr = getenv("PATH"); //recover PATH variable string
    char PathSep [] = ";"; //PATH paths separator
    #if defined __CYGWIN__
    strcpy(PathSep, ":"); //Cygwin uses linux separator
    #endif
    char tmpPath [PATH_MAX]; //full path to plosible ansicon
    char *splitPtr = strtok(envPathStr, PathSep); //pointer to "split" PATH
    while(splitPtr != NULL){ //loop thru separators
        sprintf(tmpPath, "%s\\ansicon.exe", splitPtr); //build full path string
        stat (tmpPath, &filestat);
        if (filestat.st_size > 0) { //file exist
            if(debug){fprintf(stderr,"DEBUG: ansicon.exe found : %s\n", tmpPath);}
            return 1; //success
        }
        splitPtr = strtok (NULL, PathSep); //next split
    }
    if(debug){fprintf(stderr,"DEBUG: ansicon.exe not found\n");}
    return 0; //failed
}

int checkAnsiconModule (void) { //search thru current process for ansicon module loaded
    if(debug){fprintf(stderr,"DEBUG: Start search for Ansicon module\n");}
    HANDLE currentProcess; //current process handle
    HMODULE modArr[255]; //loaded module array
    DWORD modArrSize; //size of modArr
    currentProcess = GetCurrentProcess(); //get process handle
    if (currentProcess != NULL) {
        if (EnumProcessModules (currentProcess, modArr, sizeof(modArr), &modArrSize)) { //enum all modules for current process
            for (unsigned int i = 0; i < (modArrSize / sizeof(HMODULE)); i++) {
                TCHAR modName[MAX_PATH]; //current module name
                if (GetModuleBaseName(currentProcess, modArr[i], modName, sizeof(modName) / sizeof(TCHAR))) { //get module name
                    if (strcmp(modName, "ANSI32.dll") == 0 || strcmp(modName, "ANSI64.dll") == 0) { //ansicon found
                        if(debug){fprintf(stderr,"DEBUG: Ansicon module found\n");}
                        return 1;
                    }
                }
            }
        }
        CloseHandle(currentProcess); //close current process handle
    }
    if(debug){fprintf(stderr,"DEBUG: Ansicon module not found\n");}
    return 0;
}

#else
int checkAnsiconExists (void) {return 1;} //search thru PATH for ansicon
int checkAnsiconModule (void) {return 1;} //search thru current process for ansicon module loaded
#endif
