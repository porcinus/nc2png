/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to Ansicon, also used on Windows to provide console ansi color compatibility.

upd 0.4a ok
*/

#include "ansicon.h"

#if (!defined WINCON && (defined _WIN32 || defined __CYGWIN__))
bool checkAnsiconExists(void){ //search thru PATH for ansicon
    debug_stderr("Start search for ansicon.exe in PATH variable\n");
    struct stat filestat;
    char *envPathStr = getenv("PATH"); //recover PATH variable string
    char PathSep[] = ";"; //PATH paths separator
    #if defined __CYGWIN__
    strcpy(PathSep, ":"); //Cygwin uses linux separator
    #endif
    char tmpPath[PATH_MAX]; //full path to plosible ansicon
    char *splitPtr = strtok(envPathStr, PathSep); //pointer to "split" PATH
    while(splitPtr != NULL){ //loop thru separators
        sprintf(tmpPath, "%s\\ansicon.exe", splitPtr); //build full path string
        if (stat(tmpPath, &filestat) == 0) { //file exist
            debug_stderr("ansicon.exe found : %s\n", tmpPath);
            return true; //success
        }
        splitPtr = strtok (NULL, PathSep); //next split
    }
    debug_stderr("ansicon.exe not found\n");
    return false;
}

bool checkAnsiconModule(void){ //search thru current process for ansicon module loaded
    debug_stderr("Start search for Ansicon module\n");
    HANDLE currentProcess; //current process handle
    HMODULE modArr[255]; //loaded module array
    DWORD modArrSize; //size of modArr
    currentProcess = GetCurrentProcess(); //get process handle
    if (currentProcess != NULL){
        if (EnumProcessModules (currentProcess, modArr, sizeof(modArr), &modArrSize)){ //enum all modules for current process
            for (DWORD i = 0; i < (modArrSize / sizeof(HMODULE)); i++){
                TCHAR modName[MAX_PATH]; //current module name
                if (GetModuleBaseName(currentProcess, modArr[i], modName, sizeof(modName) / sizeof(TCHAR))){ //get module name
                    if (strcmp(modName, "ANSI32.dll") == 0 || strcmp(modName, "ANSI64.dll") == 0){ //ansicon found
                        debug_stderr("Ansicon module found\n");
                        return true;
                    }
                }
            }
        }
        CloseHandle(currentProcess); //close current process handle
    }
    debug_stderr("Ansicon module not found\n");
    return false;
}
#else
bool checkAnsiconExists(void){return true;} //search thru PATH for ansicon
bool checkAnsiconModule(void){return true;} //search thru current process for ansicon module loaded
#endif

#if (defined _WIN32 || defined __CYGWIN__)
void UTF8toCP850(char* dest, char* source){ //convert utf8 chars to CP850 for proper display
    //chars map arrays based on https://www.ascii-codes.com/cp850.html and https://www.utf8-chartable.de/
    const unsigned char convTableC2[] = {255 /*NBSP*/, 173 /*¡*/, 189 /*¢*/, 156 /*£*/, 207 /*¤*/, 190 /*¥*/, 221 /*¦*/, 245 /*§*/, 249 /*¨*/, 184 /*©*/, 166 /*ª*/, 174 /*«*/, 170 /*¬*/, 240 /*­*/, 169 /*®*/, 238 /*¯*/, 248 /*°*/, 241 /*±*/, 253 /*²*/, 252 /*³*/, 239 /*´*/, 230 /*µ*/, 244 /*¶*/, 250 /*·*/, 247 /*¸*/, 251 /*¹*/, 167 /*º*/, 175 /*»*/, 172 /*¼*/, 171 /*½*/, 243 /*¾*/, 168 /*¿*/};
    const unsigned char convTableC3[] = {183 /*À*/, 181 /*Á*/, 182 /*Â*/, 199 /*Ã*/, 142 /*Ä*/, 143 /*Å*/, 146 /*Æ*/, 128 /*Ç*/, 212 /*È*/, 144 /*É*/, 210 /*Ê*/, 211 /*Ë*/, 222 /*Ì*/, 214 /*Í*/, 215 /*Î*/, 216 /*Ï*/, 209 /*Ð*/, 165 /*Ñ*/, 227 /*Ò*/, 224 /*Ó*/, 226 /*Ô*/, 229 /*Õ*/, 153 /*Ö*/, 158 /*×*/, 157 /*Ø*/, 235 /*Ù*/, 233 /*Ú*/, 234 /*Û*/, 154 /*Ü*/, 237 /*Ý*/, 232 /*Þ*/, 225 /*ß*/, 133 /*à*/, 160 /*á*/, 131 /*â*/, 198 /*ã*/, 132 /*ä*/, 134 /*å*/, 145 /*æ*/, 135 /*ç*/, 138 /*è*/, 130 /*é*/, 136 /*ê*/, 137 /*ë*/, 141 /*ì*/, 161 /*í*/, 140 /*î*/, 139 /*ï*/, 208 /*ð*/, 164 /*ñ*/, 149 /*ò*/, 162 /*ó*/, 147 /*ô*/, 228 /*õ*/, 148 /*ö*/, 246 /*÷*/, 155 /*ø*/, 151 /*ù*/, 163 /*ú*/, 150 /*û*/, 129 /*ü*/, 236 /*ý*/, 231 /*þ*/, 152 /*ÿ*/};
    char *strPtr = source, *outputPtr = dest;
    while (*strPtr != '\0'){
        if (*strPtr == '\xC2'){
            strPtr++;
            *outputPtr = (char)convTableC2[(unsigned char)*strPtr - 160];
        } else if (*strPtr == '\xC3'){
            strPtr++;
            *outputPtr = (char)convTableC3[(unsigned char)*strPtr - 128];
        } else {
            *outputPtr = *strPtr;
        }
        strPtr++; outputPtr++;
    }
    *outputPtr ='\0';
}
#else
void UTF8toCP850(char* dest, char* source){strcpy(dest, source);} //dummy
#endif

void printfTerm(const char* format, ...){ //printf clone to do UTF8 to CP850 conversion on the fly, based on printf source code
    va_list arg;
    va_start(arg, format);
#if (defined _WIN32 || defined __CYGWIN__)
    char buffer[PRINTFTERM_BUFFER_LEN], bufferOut[PRINTFTERM_BUFFER_LEN];
    vsprintf(buffer, format, arg);
    UTF8toCP850(bufferOut, buffer);
    printf(bufferOut);
#else
    vfprintf(stdout, format, arg);
#endif
    va_end(arg);
}


