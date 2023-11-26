/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

related to debug functions

upd 0.4a ok
*/

#ifndef DEBUG_H
#define DEBUG_H

#include <cstdio>

extern bool debug; //debug mode bool
extern double get_time_double(void); //get time in double (seconds)
extern double program_start_time;

#ifndef debug_stderr
    #define debug_stderr(fmt, ...) do {if(debug) fprintf(stderr, "%lf: %s:%d: %s(): " fmt, get_time_double() - program_start_time, __FILE__, __LINE__, __func__, ##__VA_ARGS__);} while (0)
#endif

#ifndef debug_stdout
    #define debug_stdout(fmt, ...) do {if(debug) fprintf(stdout, "%lf: %s:%d: %s(): " fmt, get_time_double() - program_start_time, __FILE__, __LINE__, __func__, ##__VA_ARGS__);} while (0)
#endif

#endif