/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Main file
*/

#include "ncparser.h"
#include "nc2png.h"

void show_usage () { //display usage to user
    char strBuffer [10]; //string buffer
	printf("nc2png v\033[1;32m%s\033[0m (https://github.com/porcinus/nc2png)\n", programversion);
    printf("%s : \033[1;33mnc2png Whateverfullpath/file.nc\033[0m\n", strMain[language][STR_MAIN::USAGE_EXAMPLE]);
    printf("Options:\n");
    printf("\t-cfg, %s [%s]\n", strMain[language][STR_MAIN::USAGE_NEWCFG], strMain[language][STR_MAIN::USAGE_OPTIONAL]);
    printf("\t-reset, %s [%s]\n", strMain[language][STR_MAIN::USAGE_DELCFG], strMain[language][STR_MAIN::USAGE_OPTIONAL]);
    printf("\t-debug, %s [%s]\n", strMain[language][STR_MAIN::USAGE_DEBUG], strMain[language][STR_MAIN::USAGE_OPTIONAL]);
    printf("\n%s:\n", strMain[language][STR_MAIN::USAGE_LIBRARIES]);
    printf( "- libGD (%s): https://libgd.github.io/\n"
            "- libpng (%s): http://www.libpng.org/\n"
            "- zlib (%s): https://zlib.net/\n", GD_VERSION_STRING, PNG_LIBPNG_VER_STRING, ZLIB_VERSION);
    fgets (strBuffer , 10 , stdin);
}

int main (int argc, char *argv[]) {
    char strBuffer [4096]; //string buffer
    language = getLocale (); //try to recover system language

    #if defined _WIN32 || defined __CYGWIN__
    if(checkAnsiconModule() > 0) {termColor = true; //ansicon module is found
    } else {
        if (!shouldRunAnsicon && checkAnsiconExists () != 0){ //ansicon found in PATH
            char tmpArgs [PATH_MAX];
            char tmpPath [PATH_MAX];
            GetModuleFileName(NULL, tmpPath, MAX_PATH); //get current exe fullpath
            sprintf(tmpArgs, "ansicon.exe \"%s\" -ansicon", tmpPath); //add ansicon to arguments
            for(int i = 1; i < argc; ++i){sprintf(tmpArgs, "%s %s", tmpArgs, argv[i]);} //rebuild arguments for next execution
            STARTUPINFO si; PROCESS_INFORMATION pi; ZeroMemory( &si, sizeof(si) ); si.cb = sizeof(si); ZeroMemory( &pi, sizeof(pi) ); //because needed...
            if(debug){fprintf(stderr,"DEBUG: CMD: %s\n", tmpArgs);}
            if (!CreateProcess(NULL, tmpArgs, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                if(debug){fprintf(stderr,"DEBUG: Create process failed (%d)\n", GetLastError());}
                printf("%s\n", strMain[language][STR_MAIN::ANSICON_MODULE]);
            } else {
                if(debug){fprintf(stderr,"DEBUG: New process for Ansicon created with success\n");}
            }
            return 0;
        }
    }
    #endif

    if (!termColor) { //not running in color mode
        printf("%s\n", strMain[language][STR_MAIN::ANSICON_NEEDED]);
        fgets (strBuffer , 10 , stdin); //wait for use input
        return 0; //bye
    } else {
        #if defined _WIN32 || defined __CYGWIN__
        SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, true); //handle ctrl-c on windows to avoid ansicon glitch that doesn't reset ansi code
        #endif
    }

    //dirty but working
    char programPath [PATH_MAX]; strcpy (programPath, argv[0]);
#if defined _WIN32
    char pathSep = '\\';
#else
    char pathSep = '/';
#endif
    char *tmpPtr = strrchr(programPath, pathSep);
    if (tmpPtr !=NULL){
        *(tmpPtr + 1)='\0';
        chdir (programPath); //correct current program directory to avoid multiple config files
    }

    char ncFilePath [PATH_MAX] = {'\0'}; //full path to nc file
    char ncFilename [PATH_MAX] = {'\0'}; //nc file

    configParse ();
	for(int i = 1; i < argc; ++i){ //argument to variable
		if (strcmp(argv[i], "-cfg") == 0) {configEdit (false); return 0; //config mode
		} else if (strcmp(argv[i], "-reset") == 0) {configSave (true); return 0; //reset config mode
		} else if (strcmp(argv[i], "-ansicon") == 0) {shouldRunAnsicon = true; //should run in ansicon mode
		} else if (strcmp(argv[i], "-debug") == 0) {debug = true; //debug mode
        } else if (strcmp(argv[i], "-default") == 0) { //set program as default for nc format




        } else {sprintf (ncFilePath, "%s %s", ncFilePath, argv[i]);}
	}

    if (strlen(ncFilePath) > 0) {
        struct stat filestat;
        if (ncFilePath [0] == ' ') {strcpy(ncFilePath, ncFilePath + 1);} //shift left
        stat(ncFilePath, &filestat);
        if (filestat.st_size > 0) {
            if(debug){fprintf(stderr,"DEBUG: %s found\n", ncFilePath);}
        } else {
            printf("Error: '%s' not found\n\n", ncFilePath);
            ncFilePath[0]='\0';
        }
    }

    if (strlen(ncFilePath) == 0) {show_usage(); return 0;} //no arguments, show help, quit

    configManu (); //onfly config edit without save

    //parser
    clock_t timerStart, timerEnd;
    timerStart = clock(); //start timer

    int ncLineCount = NCcountLines (ncFilePath); //count nb ob lines in nc file ot avoid overhead
    if (ncLineCount > 0) {
        if(debug){fprintf(stderr,"DEBUG: NC : line count : %d\n", ncLineCount);}

        ncFlagsStruc *ncFlags = new ncFlagsStruc [1]; //nc flags
        ncLineStruc *ncLines = new ncLineStruc [ncLineCount]; //consider num of lines in file as amount of lines in numcode
        ncToolStruc *ncDataTools = new ncToolStruc [ncToolsLimit]; //limit to 100 tools
        ncDistTimeStruc *ncDataTime = new ncDistTimeStruc [ncCommentsLimit]; //limit to 100 comments
        ncLimitStruc *ncDataLimits = new ncLimitStruc [1]; //nc limits
        ncLinesCountStruc *ncLinesCount = new ncLinesCountStruc [1]; //nc lines data

        ncArraySize *ncArraySizes = new ncArraySize; //array size limits
        ncArraySizes->lineStrucLimit = ncLineCount;
        ncArraySizes->toolStrucLimit = ncToolsLimit;
        ncArraySizes->distTimeStrucLimit = ncCommentsLimit;

        int NCparseResult = NCparseFile (ncFilePath, ncFlags, ncLines, ncDataTools, ncDataTime, ncDataLimits, ncLinesCount, ncArraySizes, debug);
        if (NCparseResult > 0) {printf ("\033[1;31mNCopenFile failed with code: %d\033[0m", NCparseResult); //failed to read nc file
        } else {
            if(debug){fprintf(stderr,"NC file parse succesfully");}
            double parserDuration = 0.;
            timerEnd = clock(); parserDuration = (double)(timerEnd - timerStart) / CLOCKS_PER_SEC;

            //parser output
            char timeArr [] = "000h 00min";
            char time1Arr [] = "000h 00min";
            double timeWork = ncLinesCount[0].totalTimeWork, timeFast = ncLinesCount[0].totalTimeFast, timeCircular = 0, timeDrill = 0;
            //for (int i=0; i<100; i++) {if (ncDataTime[i].distWork > 0 && ncDataTime[i].distFast > 0){timeWork += ncDataTime[i].timeWork; timeFast += ncDataTime[i].timeFast;}}

            tmpPtr = strrchr(ncFilePath, '/');
            if (tmpPtr != NULL) {strcpy(ncFilename, tmpPtr + 1);
            } else {tmpPtr = strrchr(ncFilePath, '\\'); if (tmpPtr != NULL) {strcpy(ncFilename, tmpPtr + 1);} else {strcpy(ncFilename, ncFilePath);}}

            printf("\n\n");
            printf (strMain[language][STR_MAIN::REPORT_L01], ncFilename);
            printf (strMain[language][STR_MAIN::REPORT_L02], speedFastXY);
            printf (strMain[language][STR_MAIN::REPORT_L03], speedFastZ);
            printf (strMain[language][STR_MAIN::REPORT_L04]);
            printf (strMain[language][STR_MAIN::REPORT_L05], ncFlags[0].coord);
            printf (strMain[language][STR_MAIN::REPORT_L06], ncFlags[0].circular);
            printf (strMain[language][STR_MAIN::REPORT_L07], ncFlags[0].unit);
            if (ncFlags[0].unit == 20) {printf (strMain[language][STR_MAIN::REPORT_L08]);} else {printf ("\n");}
            printf (strMain[language][STR_MAIN::REPORT_L09], ncFlags[0].compensation);
            if (ncFlags[0].compensation != 40) {printf (strMain[language][STR_MAIN::REPORT_L10]);} else {printf ("\n");}
            printf (strMain[language][STR_MAIN::REPORT_L11], ncFlags[0].workplane);
            if (ncFlags[0].workplane != 17) {printf (strMain[language][STR_MAIN::REPORT_L12]);} else {printf ("\n");}
            printf("\n\n");
            printf (strMain[language][STR_MAIN::REPORT_L13]);
            sec2charArr (timeArr, timeWork * 60);
            printf (strMain[language][STR_MAIN::REPORT_L14], timeArr);		
            sec2charArr (timeArr, timeFast * 60);
            printf (strMain[language][STR_MAIN::REPORT_L15], timeArr);
            sec2charArr (timeArr, (timeWork + timeFast) * 60);
            printf (strMain[language][STR_MAIN::REPORT_L16], timeArr);	
            for (unsigned int i=0; i<ncCommentsLimit; i++) {
                if (ncDataTime[i].distWork > 0 || ncDataTime[i].distFast > 0){
                    sec2charArr (timeArr, (ncDataTime[i].timeWork) * 60); sec2charArr (time1Arr, (ncDataTime[i].timeFast) * 60);
                    printf (strMain[language][STR_MAIN::REPORT_L17], ncDataTime[i].name, timeArr, time1Arr);
                }
            }

            if (speedPercent < 100) {
                printf("\n\n");
                printf (strMain[language][STR_MAIN::REPORT_L18], speedPercent);
                sec2charArr (timeArr, (timeWork / ((double)speedPercent / 100)) * 60);
                printf (strMain[language][STR_MAIN::REPORT_L19], timeArr);		
                sec2charArr (timeArr, (timeFast / ((double)speedPercent / 100)) * 60);
                printf (strMain[language][STR_MAIN::REPORT_L20], timeArr);
                sec2charArr (timeArr, ((timeWork + timeFast) / ((double)speedPercent / 100)) * 60);
                printf (strMain[language][STR_MAIN::REPORT_L21], timeArr);	
                for (unsigned int i=0; i<ncCommentsLimit; i++) {
                    if (ncDataTime[i].distWork > 0 || ncDataTime[i].distFast > 0){
                        sec2charArr (timeArr, (ncDataTime[i].timeWork / ((double)speedPercent / 100)) * 60); sec2charArr (time1Arr, (ncDataTime[i].timeFast / ((double)speedPercent / 100)) * 60);
                        printf (strMain[language][STR_MAIN::REPORT_L22], ncDataTime[i].name, timeArr, time1Arr);
                    }
                }
            }
            printf("\n\n");
            printf (strMain[language][STR_MAIN::REPORT_L23]);
            printf (strMain[language][STR_MAIN::REPORT_L24], ncDataLimits[0].xMin, ncDataLimits[0].xMax, ncDataLimits[0].xMax - ncDataLimits[0].xMin);
            printf (strMain[language][STR_MAIN::REPORT_L25], ncDataLimits[0].yMin, ncDataLimits[0].yMax, ncDataLimits[0].yMax - ncDataLimits[0].yMin);
            printf (strMain[language][STR_MAIN::REPORT_L26], ncDataLimits[0].zMin, ncDataLimits[0].zMax, ncDataLimits[0].zMax - ncDataLimits[0].zMin);
            printf("\n\n");
            printf (strMain[language][STR_MAIN::REPORT_L27]);
            timeWork = timeFast = timeCircular = timeDrill = 0; for (unsigned int i=0; i<ncCommentsLimit; i++) {if (ncDataTime[i].distWork > 0 || ncDataTime[i].distFast > 0){timeWork += ncDataTime[i].distWork; timeFast += ncDataTime[i].distFast; timeCircular += ncDataTime[i].distCircular; timeDrill += ncDataTime[i].distDrill;}}
            printf (strMain[language][STR_MAIN::REPORT_L28], timeWork);
            printf (strMain[language][STR_MAIN::REPORT_L29], timeFast);
            printf (strMain[language][STR_MAIN::REPORT_L30], timeCircular);
            printf (strMain[language][STR_MAIN::REPORT_L31], timeDrill);
            printf (strMain[language][STR_MAIN::REPORT_L32], timeWork + timeFast);
            printf("\n\n");
            printf (strMain[language][STR_MAIN::REPORT_L33]);
            printf (strMain[language][STR_MAIN::REPORT_L34], ncLinesCount[0].all);
            printf (strMain[language][STR_MAIN::REPORT_L35], ncLinesCount[0].commented);
            printf (strMain[language][STR_MAIN::REPORT_L36], ncLinesCount[0].skip);
            printf (strMain[language][STR_MAIN::REPORT_L37], ncLinesCount[0].g0);
            printf (strMain[language][STR_MAIN::REPORT_L38], ncLinesCount[0].g1);
            printf (strMain[language][STR_MAIN::REPORT_L39], ncLinesCount[0].g2);
            printf (strMain[language][STR_MAIN::REPORT_L40], ncLinesCount[0].g3);
            printf (strMain[language][STR_MAIN::REPORT_L41], ncLinesCount[0].g81);
            
            printf("\n\n");
            printf(strMain[language][STR_MAIN::GENTIME_NC], parserDuration);

            //gd preview
            timerStart = clock();
            int gdPreviewResult = gdPreview (ncFilePath, gdWidth, gdArcRes, ncFlags, ncLines, ncDataTools, ncDataTime, ncDataLimits, ncLinesCount, debug);
            if (NCparseResult > 0) {printf ("\033[1;gdPreview failed with code: %d\033[0m", gdPreviewResult); //failed to generate gd preview
            } else {
                double previewDuration = 0.;
                timerEnd = clock(); previewDuration = (double)(timerEnd - timerStart) / CLOCKS_PER_SEC;
                printf(strMain[language][STR_MAIN::GENTIME_GD], previewDuration);
            }
        }

        delete ncFlags;
        delete ncLines;
        delete ncDataTools;
        delete ncDataTime;
        delete ncDataLimits;
        delete ncArraySizes;
        delete ncLinesCount;
    } else {printf ("\033[1;31mFailed to read NC file, 0 line read\033[0m");} //failed to read nc file

    fgets (strBuffer , 10 , stdin); //wait for user input
    return 0;
}
