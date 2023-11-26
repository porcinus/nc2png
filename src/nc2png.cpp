/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Main file
*/

#include "nc2png_language.h"
#include "ncparser.h"
#include "nc2png.h"


void showUsage(void){ //display usage to user
	printfTerm("nc2png v\033[1;32m%s\033[0m (https://github.com/porcinus/nc2png)\n", programversion);
    printfTerm("%s : \033[1;33mnc2png Whateverfullpath/file.nc\033[0m\n", strUsageTerm[STR_USAGE::USAGE_EXAMPLE][language]);
    printfTerm("Options:\n");
    printfTerm("\t-cfg-new, %s [%s]\n", strUsageTerm[STR_USAGE::USAGE_CFGNEW][language], strUsageTerm[STR_USAGE::USAGE_OPTIONAL][language]);
    printfTerm("\t-cfg-edit, %s [%s]\n", strUsageTerm[STR_USAGE::USAGE_CFGEDIT][language], strUsageTerm[STR_USAGE::USAGE_OPTIONAL][language]);
    printfTerm("\t-debug, %s [%s]\n", strUsageTerm[STR_USAGE::USAGE_DEBUG][language], strUsageTerm[STR_USAGE::USAGE_OPTIONAL][language]);
    printfTerm("\n%s:\n", strUsageTerm[STR_USAGE::USAGE_LIBRARIES][language]);
    printfTerm( "- libGD (%s): https://libgd.github.io/\n"
            "- libpng (%s): http://www.libpng.org/\n"
            "- zlib (%s): https://zlib.net/\n", GD_VERSION_STRING, PNG_LIBPNG_VER_STRING, ZLIB_VERSION);
    printfTerm("\nOpenGL:\n");
    printfTerm( "- GLFW: https://www.glfw.org/\n"
            "- GLAD: https://glad.dav1d.de/\n"
            "- GLM: https://github.com/g-truc/glm\n"
            "- ImGui: https://github.com/ocornut/imgui\n"
            "- Huge thanks: https://learnopengl.com/Introduction\n");
    printf("\n\n");
}


double get_time_double(void){ //get time in double (seconds)
    struct timespec tp; int result = clock_gettime(CLOCK_MONOTONIC, &tp);
    if (result == 0) {return tp.tv_sec + (double)tp.tv_nsec/1e9;}
    return -1.; //failed
}


bool userInput(bool alwaysTrue){ //read stdin, return true when return char detected, false overwise
    bool ret = alwaysTrue;
    for (unsigned int i = 0; i < INPUT_BUFFER_SIZE; i++){inputBuffer[i] = '\0';} //reset whole input buffer
    debug_stderr("start user input\n");
    fgets(inputBuffer, INPUT_BUFFER_SIZE - 1, stdin);
    unsigned int strLen = strlen(inputBuffer);
    if (inputBuffer[strLen - 1] == 0x0A){
        inputBuffer[strLen - 1] = '\0'; //remove return char
        ret = true;
    }
    debug_stderr("buffer :'%s'\n", inputBuffer);
    debug_stderr("return %d\n", ret ? 1 : 0);
    return ret;
}


bool pressEnterClose(void){ //tty close prompt, returns userInput return
    printfTerm("%s\n", strGeneric[STR_GENERIC::GENERIC_PRESSKEYCLOSE][language]);
    return userInput(true);
}


bool pressEnterContinue(void){ //tty continue prompt, returns userInput return
    printfTerm("%s\n", strGeneric[STR_GENERIC::GENERIC_PRESSKEYCONTINUE][language]);
    return userInput(false);
}


bool pressEnterSave(void){ //tty save prompt, returns userInput return
    printfTerm("%s\n", strConfigEditTerm[STR_CONFIG_EDIT_TERM::TERM_SAVE_CONFIRM][language]);
    return userInput(false);
}


void configEditTerm(void){ //new settings screen
    char strBuffer[100];
    for (unsigned int i = 0; i < cfg_vars_arr_size; i++){
        bool cfg_vars_to_skip = false;
        for (unsigned int j = 0; j < cfg_vars_skip_size; j++){
            if (strcmp(cfg_vars_skip[j], cfg_vars[i].name) == 0){
                cfg_vars_to_skip = true;
                break;
            }
        }

        if (!cfg_vars_to_skip){
            config_value_to_str(strBuffer, 99, cfg_vars[i].type, cfg_vars[i].ptr);
            printfTerm("\033[0m%s (\033[1;33m%s\033[0m): \n > \033[1;32m", strConfigItem[i][language], strBuffer);
            if (userInput(false)){ //user entered a value
                config_type_parse(cfg_vars, cfg_vars_arr_size, i, cfg_vars[i].type, (char*)cfg_vars[i].name, inputBuffer);
            } else {exit(0);}
        }
    }
    
    printfTerm("\033[0m\n\033[1m%s\033[0m\n", strConfigEditTerm[STR_CONFIG_EDIT_TERM::TERM_NEW_SETTINGS][language]);
    for (unsigned int i = 0; i < cfg_vars_arr_size; i++){
        config_value_to_str(strBuffer, 99, cfg_vars[i].type, cfg_vars[i].ptr);
        printfTerm("\033[0m%s > \033[1;32m%s\033[0m\n", strConfigItem[i][language], strBuffer);
    }
    printf("\n");
    if (pressEnterSave()){
        config_save(cfg_vars, cfg_vars_arr_size, (char*)cfg_filename, -1, -1, false); //save new config
    } else {exit(0);} //quit program
}


void configManuTerm(char** varsList, unsigned int varCount){ //manual settings screen
    char strBuffer[100];
    for (unsigned int i = 0; i < varCount; i++){
        if (varsList[i] == NULL){continue;}
        int cfgIndex = config_search_name(cfg_vars, cfg_vars_arr_size, varsList[i], true); //get index from config var
        if (cfgIndex != -1){
            config_value_to_str(strBuffer, 99, cfg_vars[cfgIndex].type, cfg_vars[cfgIndex].ptr);
            printfTerm("\033[0m%s (\033[1;33m%s\033[0m): \n > \033[1;32m", strConfigItem[cfgIndex][language], strBuffer);
            if (userInput(false)){ //user entered a value
                config_type_parse(cfg_vars, cfg_vars_arr_size, cfgIndex, cfg_vars[cfgIndex].type, (char*)cfg_vars[cfgIndex].name, inputBuffer);
            } else {exit(0);}
        }
    }
    printf("\n");
}


#if (defined _WIN32 || defined __CYGWIN__)
    BOOL WINAPI CtrlHandler(DWORD signal){ //handle ctrl-c on windows to avoid ansicon glitch that doesn't reset ansi code
        if (signal == CTRL_C_EVENT){
            debug_stderr("Windows: Ctrl+C triggered\n");
            //programClose();
            exit(0);
        }
        return true;
    }
#else
    void programSignal(int signal){ //program received a signal that kills it
        debug_stderr("signal received:%d\n", signal);
        exit(0);
    }
#endif


void programClose(void){ //run when program closes
    //todo align failed return code to errno
    if (alreadyKilled){return;}
    usleep(100000); //avoid potential garbage on tty output
    if (termColor){printf("\033[0m");} //reset ansi formatting
    
    //free nc parser vars
    debug_stderr("memory cleanup\n");
    if (ncFlags != nullptr){delete []ncFlags; ncFlags = nullptr;} //g flags
    if (ncLines != nullptr){delete []ncLines; ncLines = nullptr;} //lines data
    if (ncDataTools != nullptr){delete []ncDataTools; ncDataTools = nullptr;} //tools data
    if (ncComments != nullptr){delete []ncComments; ncComments = nullptr;} //operations time
    
    alreadyKilled = true;
}

int main(int argc, char* argv[]){
    program_start_time = get_time_double(); //program start time, mainly used for debug

    atexit(programClose); //run on program exit

    //avoid run loop if ansicon not installed in windows, or terminal detection failed
    bool programRestarted = false;
	for (int i = 1; i < argc; ++i){
        if (strcmp(argv[i], "-restarted") == 0){
            programRestarted = true; //should be run in ansicon mode or in a new terminal
            argv[i][0] = '\0';
		} else if (strcmp(argv[i], "-debug") == 0){
            debug = true; //debug mode
            argv[i][0] = '\0';
        }
	}
    debug_stderr("programRestarted:%d\n", programRestarted ? 1 : 0);
    
    //terminal signal handling
    #if (defined(_WIN32) || defined(__CYGWIN__))
        SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, true); //handle ctrl-c on windows to avoid ansicon glitch that doesn't reset ansi code
    #else
        signal(SIGINT, programSignal); //ctrl-c
        signal(SIGTERM, programSignal); //SIGTERM from htop or other, SIGKILL not work as program get killed before able to handle
        signal(SIGABRT, programSignal); //failure
    #endif
    
    //try to get system language
    language = getLocale();
    
    #if (defined _WIN32 || defined __CYGWIN__)
        #ifndef WINCON
            //ansicon allow ansi escape code compat with older windows systems
            if(checkAnsiconModule() > 0){termColor = true; //ansicon module is found
            } else if (!programRestarted && checkAnsiconExists() != 0){ //ansicon found in PATH
                char tmpArgs[PATH_MAX];
                char tmpPath[PATH_MAX];
                GetModuleFileName(NULL, tmpPath, MAX_PATH); //get current exe fullpath
                sprintf(tmpArgs, "ansicon.exe \"%s\" -restarted", tmpPath); //add ansicon to arguments
                for(int i = 1; i < argc; ++i){sprintf(tmpArgs, "%s %s", tmpArgs, argv[i]);} //rebuild arguments for next execution
                STARTUPINFO si; PROCESS_INFORMATION pi; ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si); ZeroMemory(&pi, sizeof(pi)); //because needed...
                debug_stderr("cmd: %s\n", tmpArgs);
                if (!CreateProcess(NULL, tmpArgs, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                    debug_stderr("create process failed (%ld)\n", GetLastError());
                    printfTerm("%s\n", strAnsi[STR_ANSI::ANSICON_MODULE][language]);
                    pressEnterClose();
                } else {
                    debug_stderr("new process for Ansicon created with success\n");
                }
                return 0;
            }
        #endif
    #else
        //doesn't look to be running from terminal
        if (!programRestarted && !runningFromTerminal()){
            const char* terminal = terminalFilePath();
            if (terminal == NULL){
                debug_stderr("failed to detect system terminal emulator\n");
                return 0;
            }
            char tmpArgs[PATH_MAX];
            sprintf(tmpArgs, "%s \"'%s' -restarted", terminal, argv[0]);
            for(int i = 1; i < argc; ++i){sprintf(tmpArgs, "%s %s", tmpArgs, argv[i]);} //rebuild arguments for next execution
            strcat(tmpArgs, "\" &"); //fork
            system(tmpArgs);
            sleep(1); //wait a bit to avoid killing initial program before command is running
            return 0;
        }
    #endif

    //not running in color mode
    #if (!defined WINCON && (defined _WIN32 || defined __CYGWIN__))
        if (!termColor){
            printfTerm("%s\n", strAnsi[STR_ANSI::ANSI_REQUIRED][language]);
            pressEnterClose();
            return 0;
        }
    #endif

    { //get program path and change current working folder
        char programPath[strlen(argv[0]) + 3]; strcpy(programPath, argv[0]);
        #if defined _WIN32
            char *tmpPtr = strrchr(programPath, '\\');
        #else
            char *tmpPtr = strrchr(programPath, '/');
        #endif
        if (tmpPtr != NULL){*(tmpPtr + 1) = '\0'; chdir(programPath);}
    }
    
    //parse program arguments
    char ncFilePath[PATH_MAX] = "", ncFilename[PATH_MAX] = ""; //full path/filename of nc file

	for(int i = 1; i < argc; ++i){
		if (strcmp(argv[i], "-cfg-new") == 0){configEditTerm(); return 0; //new config
		} else if (strcmp(argv[i], "-cfg-edit") == 0){ //edit config
            if (i + 1 < argc && config_set(cfg_vars, cfg_vars_arr_size, (char*)cfg_filename, -1, -1, true, argv[++i]) == 0){ //update var in config file
                config_save(cfg_vars, cfg_vars_arr_size, (char*)cfg_filename, -1, -1, false); //save new config
            } else {
                printfTerm("%s\n", strConfigEditTerm[STR_CONFIG_EDIT_TERM::TERM_INVALID_EDIT_FORMAT][language]); //invalid format
            }
            return 0;
        } else if (argv[i][0] != '\0'){sprintf(ncFilePath, "%s %s", ncFilePath, argv[i]);}
	}
    
    //parse/create config file
    if (config_parse(cfg_vars, cfg_vars_arr_size, (char*)cfg_filename, -1, -1, false) < 0){
        configEditTerm();
        return 0;
    }
    
    //if (strlen(ncFilePath) < 1){strcpy(ncFilePath, "Z:\\Work\\Programmation\\VSCode\\C++\\nc2png_local\\testnc mm.nc");}

    //path to nc file provided
    if (strlen(ncFilePath) > 0){
        while (ncFilePath[0] == ' '){memmove(ncFilePath, ncFilePath + 1, strlen(ncFilePath));} //trim starting space(s)
        struct stat fileStats;
        if (stat(ncFilePath, &fileStats) == 0){
            debug_stderr("%s found\n", ncFilePath);
        } else {
            printfTerm(strGeneric[STR_GENERIC::GENERIC_FILENOTFOUND][language], ncFilePath); printf("\n\n");
            ncFilePath[0] = '\0';
        }
    }
    
    //no arguments or invalid file, show help, quit
    if (strlen(ncFilePath) == 0){
        showUsage();
        pressEnterClose();
        return 0;
    } else {
        //extract filename
        char *tmpPtr = strrchr(ncFilePath, '/');
        if (tmpPtr != NULL){strcpy(ncFilename, tmpPtr + 1);
        } else {
            tmpPtr = strrchr(ncFilePath, '\\');
            strcpy(ncFilename, (tmpPtr != NULL) ? tmpPtr + 1 : ncFilePath);
        }
        
        //manual settings screen
        if (!skipCncPrompt){
            #define manuListCount 3
            const char* manuList[manuListCount] = {"speedFastXY", "speedFastZ", "speedPercent"}; //vars allow to edit
            configManuTerm((char**)manuList, manuListCount);
        }
        program_start_time = get_time_double(); //reset program start time
    }
    
    //quick check nc file content
    unsigned int ncLineCount = ncCountLines(ncFilePath); //count nb of lines in nc file to avoid overhead
    if (ncLineCount == 0){ //empty file
        printfTerm(strGeneric[STR_GENERIC::GENERIC_FILEISEMPTY][language], ncFilePath); printf("\n\n");
        pressEnterClose();
        return 0;
    } else {debug_stderr("nc lines count : %d\n", ncLineCount);}
    
    //allocate memory
    ncFlags = new(std::nothrow)ncFlagsStruct; //g flags
    ncLines = new(std::nothrow)ncLineStruct[ncLineCount]; //lines data
    ncDataTools = new(std::nothrow)ncToolStruct[ncToolsLimit]; //tools data
    ncComments = new(std::nothrow)ncCommentStruct[ncCommentsLimit]; //operations time
    if (ncFlags == nullptr || ncLines == nullptr || ncDataTools == nullptr || ncComments == nullptr){
        printfTerm(strGeneric[STR_GENERIC::GENERIC_MEMALLOCFAILED][language], ncFilePath); printf("\n\n");
        pressEnterClose();
        return 0;
    } else {
        debug_stderr("ncFlags addr : 0x%p\n", (void *)ncFlags);
        debug_stderr("ncLines addr : 0x%p\n", (void *)ncLines);
        debug_stderr("ncDataTools addr : 0x%p\n", (void *)ncDataTools);
        debug_stderr("ncComments addr : 0x%p\n", (void *)ncComments);
    }
    ncLimitStruct ncDataLimits = {0}; //limits
    ncSummaryStruct ncSummary = {0}; //lines count
    ncArraySize ncArraySizes = {.lineStrucLimit = ncLineCount, .commentStrucLimit = ncCommentsLimit, .toolStrucLimit = ncToolsLimit,}; //array size limits
    
    //parser
    double durationBench = get_time_double();
    int result = ncParseFile(ncFilePath, speedFastXY, speedFastZ, ncFlags, ncLines, ncDataTools, ncComments, &ncDataLimits, &ncSummary, &ncArraySizes, debugGcode);
    durationBench = get_time_double() - durationBench;
    
    debug_stderr("ncLineCount:%u, ncArraySizes.lineStrucLimit:%u\n", ncLineCount, ncArraySizes.lineStrucLimit);
    
    if (result != 0){
        printfTerm(strParserTerm[STR_FAILED_TERM::TERM_PARSER_FAILED][language], result); printf("\n\n");
        pressEnterClose();
        return 0;
    }
    
    //nc parser info report
    ncParseReportTerm(ncFilename, speedFastXY, speedFastZ, speedPercent, ncFlags, ncDataTools, ncComments, &ncDataLimits, &ncSummary, &ncArraySizes);
    printfTerm(strBenchmarkReportTerm[STR_BENCHMARK_REPORT::BENCHMARK_PARSER][language], durationBench);
    
    //png preview
    if (gdEnable){
        double durationBench = get_time_double();
        result = gdPreview(ncFilePath, gdWidth, gdArcRes, gdExportDepthMap, ncLines, ncDataTools, ncComments, &ncDataLimits, &ncSummary, &ncArraySizes, debugGD);
        durationBench = get_time_double() - durationBench;
        
        if (result != 0){
            printfTerm(strParserTerm[STR_FAILED_TERM::TERM_GD_FAILED][language], result); printf("\n\n");
            pressEnterClose();
            return 0;
        }
        
        printfTerm(strBenchmarkReportTerm[STR_BENCHMARK_REPORT::BENCHMARK_GD][language], durationBench);
    }
    
    //svg preview
    if (svgEnable){
        double durationBench = get_time_double();
        result = svgPreview(ncFilePath, svgPrevArcRes, ncLines, ncDataTools, ncComments, &ncDataLimits, &ncSummary, &ncArraySizes, debugSVG);
        durationBench = get_time_double() - durationBench;
        
        if (result != 0){
            printfTerm(strParserTerm[STR_FAILED_TERM::TERM_SVG_FAILED][language], result); printf("\n\n");
            pressEnterClose();
            return 0;
        }
        
        printfTerm(strBenchmarkReportTerm[STR_BENCHMARK_REPORT::BENCHMARK_SVG][language], durationBench);
    }
    
    //opengl viewport
    if (glViewportEnable){
        result = glPreview(ncFilePath, ncLines, ncDataTools, ncComments, &ncDataLimits, &ncSummary, &ncArraySizes, debugOpenGL);
        if (result != 0){
            printfTerm(strParserTerm[STR_FAILED_TERM::TERM_OPENGL_FAILED][language], result); printf("\n\n");
            goto majorFailure;
        }
        return 0;
    }
    
    majorFailure:;
    pressEnterClose();
    return 0;
}
