/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

NNS configuration file handler
Related to configuration functions/vars.

upd 0.4a ok
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include "lang.h"
#include "debug.h"

//file newline
#if ((defined(_WIN32) || defined(__CYGWIN__)) && !defined(TXTNL))
    #define TXTNL "\r\n"
#else
    #define TXTNL "\n"
#endif

//structs
typedef struct cfg_vars_struct {
	const char* name; //var name
	const char* desc; //var description, comment at the end of the line
	int type; //0:int, 1:uint, 2:float, 3:double, 4:bool, 5:int array (split by comma in cfg file), 6:hex8, 7:hex16, 8:hex32, 9:bin8, 10:bin16, 11:bin32
	void* ptr; //pointer to value
} cfg_vars_t;

//funcs
int config_sum(cfg_vars_t* cfg, unsigned int cfg_size); //pseudo checksum for config build
int config_search_name(cfg_vars_t* cfg, unsigned int cfg_size, char* value, bool skipNl); //search in cfg_vars struct, return index if found, -1 if not
int config_save(cfg_vars_t* cfg, unsigned int cfg_size, char* filename, int uid, int gid, bool reset); //save config file
int config_set(cfg_vars_t* cfg, unsigned int cfg_size, char* filename, int uid, int gid, bool readcfg, char* var_value); //update var based on config file
bool config_type_parse(cfg_vars_t* cfg, unsigned int cfg_size, unsigned int index, int type, char* var, char* value); //parse config var with specific type
bool config_value_to_str(char* str, unsigned int strSize, int type, void* value); //convert void pointer to char array string using given cfg_vars_struct type
int config_parse(cfg_vars_t* cfg, unsigned int cfg_size, char* filename, int uid, int gid, bool createFile); //parse/create program config file, return -errno on failure, 0 on success, 1 if new config created
void config_list(cfg_vars_t* cfg, unsigned int cfg_size); //print all config vars

//extern funcs
extern void printfTerm(const char* format, ...); //ansicon.h: printf clone to do UTF8 to CP850 on the fly, based on printf source code



#endif