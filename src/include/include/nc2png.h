/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Main file

upd 0.4a ok
*/

#ifndef NC2PNG_H
#define NC2PNG_H

//default vars
bool debug = 0; //debug mode bool
bool debugGcode = 0; //output gcode debug file bool
bool debugGD = 0; //output gd debug file bool
bool debugSVG = 0; //output svg debug file bool
bool debugOpenGL = 0; //output opengl debug file bool

unsigned int speedFastXY = 2500; //max XY moving speed
unsigned int speedFastZ = 1000;  //max Z moving speed
unsigned int speedPercent = 100; //max running percent speed, used as a override
bool skipCncPrompt = false; //skip cnc settings prompt before generating priviews

unsigned int gdWidth = 800; //max grid width/height in px
unsigned int gdArcRes = 2; //arc drawing resolution
bool gdExportDepthMap = false; //export gd depthmap is cutview tools detected
bool gdEnable = true; //enable png output

unsigned int svgEnable = true; //enable svg output
float svgPrevArcRes = 1.0f; //arc resolution

bool glViewportEnable = true; //enable opengl viewport
unsigned int glViewportWidth = 800, glViewportHeight = 600; //viewport width/height
float glDrawArcPrecision = 1.0f; //arc resolution
float glHeightMapPrecision = 0.5f; //1px per given unit

unsigned int ncCommentsLimit = NCPARSER_COMMENT_ARRAY; //gcode comments array limit
unsigned int ncToolsLimit = 100; //gcode tools array limit

//program
char programversion[]="0.4a"; //program version


#include <cstring>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <new>
#include <errno.h>
extern "C" {
#include <png.h>
#include <zlib.h>
#include "gd.h"
}


#if (defined _WIN32 || defined __CYGWIN__)
    #include <windows.h>
    BOOL WINAPI CtrlHandler(DWORD signal); //handle ctrl-c on windows to avoid ansicon glitch that doesn't reset ansi code
    extern bool checkAnsiconExists(void); //search thru PATH for ansicon
    extern bool checkAnsiconModule(void); //search thru current process for ansicon module loaded
#else
    #include <signal.h>
    void programSignal(int signal); //program received a signal that kills it
    extern bool runningFromTerminal(void); //try to detect if program running from terminal
    extern const char* terminalFilePath(void); //try to get proper terminal executable file
#endif

#include "debug.h"
#include "config.h"


//user input
#define INPUT_BUFFER_SIZE 128
char inputBuffer[INPUT_BUFFER_SIZE];

//funcs
double get_time_double(void); //get time in double (seconds)
bool userInput(bool alwaysTrue); //read stdin, return true when return char detected, false overwise
bool pressEnterClose(void); //tty close prompt, returns userInput return
bool pressEnterContinue(void); //tty continue prompt, returns userInput return
bool pressEnterSave(void); //tty save prompt, returns userInput return
void showUsage(void); //display usage to user
void configEditTerm(void); //new settings screen
void configManuTerm(char** varsList, unsigned int varCount); //manual settings screen
void programClose(void); //run when program closes

//extern funcs
extern void printfTerm(const char* format, ...); //ansicon.h: printf clone to do UTF8 to CP850 on the fly, based on printf source code
extern int gdPreview(char* file, int gdPrevWidth, int gdPrevArcRes, bool properDepthMap, ncLineStruct* lines, ncToolStruct* tools, ncCommentStruct* comments, ncLimitStruct* limits, ncSummaryStruct* summary, ncArraySize* arrSizes, bool debugOutput); //generate image preview from ncparser data
extern int svgPreview(char* file, int svgPrevArcRes, ncLineStruct* lines, ncToolStruct* tools, ncCommentStruct* comments, ncLimitStruct* limits, ncSummaryStruct* summary, ncArraySize* arrSizes, bool debugOutput); //generate image preview from ncparser data
extern int glPreview(char* file, ncLineStruct* lines, ncToolStruct* tools, ncCommentStruct* comments, ncLimitStruct* limits, ncSummaryStruct* summary, ncArraySize* arrSizes, bool debugOutput);

//vars
bool alreadyKilled = false; //program already killed
unsigned int language = 0; //default language id
double program_start_time = .0; //mainly used for debug
unsigned int streamBufferSize = 32768; //try to optimize file writing using bigger buffer

#if (defined __linux__ || defined __APPLE__ || defined WINCON)
    bool termColor = true;
#else
    bool termColor = false;
#endif

//nc
ncFlagsStruct *ncFlags = nullptr; //g flags
ncLineStruct *ncLines = nullptr; //lines data
ncToolStruct *ncDataTools = nullptr; //tools data
ncCommentStruct *ncComments = nullptr; //operations time

//config file vars
const char cfg_filename[] = "nc2png.cfg"; //path to config file
cfg_vars_t cfg_vars[] = { //comments only accept one language
    {"debug", strConfigItem[STR_CONFIG_ITEM_TERM::DEBUG_TERM][language], 4, &debug},
    {"debugGcode", strConfigItem[STR_CONFIG_ITEM_TERM::DEBUG_GCODE_TERM][language], 4, &debugGcode},
    {"debugGD", strConfigItem[STR_CONFIG_ITEM_TERM::DEBUG_GD_TERM][language], 4, &debugGD},
    {"debugSVG", strConfigItem[STR_CONFIG_ITEM_TERM::DEBUG_SVG_TERM][language], 4, &debugSVG},
    {"debugOpenGL", strConfigItem[STR_CONFIG_ITEM_TERM::DEBUG_OPENGL_TERM][language], 4, &debugOpenGL},
    {TXTNL "speedFastXY", strConfigItem[STR_CONFIG_ITEM_TERM::SPEED_XY_TERM][language], 1, &speedFastXY},
    {"speedFastZ", strConfigItem[STR_CONFIG_ITEM_TERM::SPEED_Z_TERM][language], 1, &speedFastZ},
    {"speedPercent", strConfigItem[STR_CONFIG_ITEM_TERM::SPEED_PERCENT_TERM][language], 1, &speedPercent},
    {"skipCncPrompt", strConfigItem[STR_CONFIG_ITEM_TERM::SKIP_CNC_PROMPT][language], 4, &skipCncPrompt},
    {TXTNL "gdEnable", strConfigItem[STR_CONFIG_ITEM_TERM::GD_EN_TERM][language], 4, &gdEnable},
    {"gdWidth", strConfigItem[STR_CONFIG_ITEM_TERM::GD_WIDTH_TERM][language], 1, &gdWidth},
    {"gdArcRes", strConfigItem[STR_CONFIG_ITEM_TERM::GD_ARCRES_TERM][language], 1, &gdArcRes},
    {"gdExportDepthMap", strConfigItem[STR_CONFIG_ITEM_TERM::GD_EXPORTDEPTHMAP_TERM][language], 4, &gdExportDepthMap},
    {TXTNL "svgEnable", strConfigItem[STR_CONFIG_ITEM_TERM::SVG_EN_TERM][language], 4, &svgEnable},
    {"svgArcRes", strConfigItem[STR_CONFIG_ITEM_TERM::SVG_ARCRES_TERM][language], 2, &svgPrevArcRes},
    {TXTNL "glViewportEnable", strConfigItem[STR_CONFIG_ITEM_TERM::GL_EN_TERM][language], 4, &glViewportEnable},
    {"glViewportWidth", strConfigItem[STR_CONFIG_ITEM_TERM::GL_WIDTH_TERM][language], 1, &glViewportWidth},
    {"glViewportHeight", strConfigItem[STR_CONFIG_ITEM_TERM::GL_HEIGHT_TERM][language], 1, &glViewportHeight},
    {"glDrawArcPrecision", strConfigItem[STR_CONFIG_ITEM_TERM::GL_ARCRES_TERM][language], 2, &glDrawArcPrecision},
    {"glHeightMapPrecision", strConfigItem[STR_CONFIG_ITEM_TERM::GL_HEIGHTMAPRES_TERM][language], 2, &glHeightMapPrecision},
};
const unsigned int cfg_vars_arr_size = sizeof(cfg_vars) / sizeof(cfg_vars[0]); //config array size

const unsigned int cfg_vars_skip_size = 5;
const char* cfg_vars_skip[cfg_vars_skip_size] = {"debug", "debugGcode", "debugGD", "debugSVG", "debugOpenGL"};


//avoid linker glitch
unsigned int cfg_vars_arr_size_tmp = cfg_vars_arr_size;
const char* cfg_filename_tmp = cfg_filename;

#endif