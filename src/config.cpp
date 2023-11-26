/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

NNS configuration file handler
Related to configuration functions/vars.

upd 0.4a ok
*/

#include "config_language.h"
#include "config.h"

int config_sum(cfg_vars_t* cfg, unsigned int cfg_size){ //pseudo checksum for config build
    int ret = 0;
    for (unsigned int i = 0; i < cfg_size; i++){ret += strlen(cfg[i].name) + (cfg[i].type + 1)*2 + i*4;}
    return ret;
}

int config_search_name(cfg_vars_t* cfg, unsigned int cfg_size, char* value, bool skipNl){ //search in cfg_vars struct, return index if found, -1 if not
    for (unsigned int i = 0; i < cfg_size; i++){
        char *rowPtr = (char*)cfg[i].name;
        while (*rowPtr == ' ' || (skipNl && (*rowPtr == '\r' || *rowPtr == '\n'))){rowPtr++;} //skip space and new line chars at beginning of var
        if (strcmp(rowPtr, value) == 0){return i;}
    }
    return -1;
}

int config_save(cfg_vars_t* cfg, unsigned int cfg_size, char* filename, int uid, int gid, bool reset){ //save config file
    if (reset && remove(filename) != 0){debug_stderr("failed to delete '%s'\n", filename);}
    FILE *filehandle = fopen(filename, "wb");
    if (filehandle != NULL){
        for (unsigned int i = 0; i < cfg_size; i++){
            int tmpType = cfg[i].type;
            if (tmpType == 0){fprintf(filehandle, "%s=%d;", cfg[i].name, *(int*)cfg[i].ptr); //int
            } else if (tmpType == 1){fprintf(filehandle, "%s=%u;", cfg[i].name, *(unsigned int*)cfg[i].ptr); //unsigned int
            } else if (tmpType == 2){fprintf(filehandle, "%s=%f;", cfg[i].name, *(float*)cfg[i].ptr); //float
            } else if (tmpType == 3){fprintf(filehandle, "%s=%lf;", cfg[i].name, *(double*)cfg[i].ptr); //double
            } else if (tmpType == 4){fprintf(filehandle, "%s=%d;", cfg[i].name, (*(bool*)cfg[i].ptr)?1:0); //bool
            } else if (tmpType == 5){ //int array, output format: var=%d,%d,%d,...;
                unsigned int arrSize = *(int*)((int*)cfg[i].ptr)[1];
                char strBuffer[4096], strBuffer1[33];
                sprintf(strBuffer, "%s=", cfg[i].name); int strBufferSize = strlen(strBuffer);
                for (unsigned int j = 0; j < arrSize; j++){
                    strBufferSize += sprintf(strBuffer1, "%d,", ((int*)((int*)cfg[i].ptr)[0])[j]);
                    strcat (strBuffer, strBuffer1);
                }
                *(strBuffer+strBufferSize-1) = '\0';
                fprintf(filehandle, "%s;", strBuffer);
            } else if (tmpType >= 6 && tmpType < 9){ //hex8-32
                char strBuffer[4096];
                int ind = 2; for(int j=0; j<tmpType-6; j++){ind*=2;}
                sprintf(strBuffer, "%%s=0x%%0%dX;", ind); //build that way to limit var size
                fprintf(filehandle, strBuffer, cfg[i].name, *(int*)cfg[i].ptr);
            } else if (tmpType >= 9 && tmpType < 12){ //bin8-32 (itoa bypass)
                char strBuffer[4096];
                int ind = 8; for(int j=0; j<tmpType-9; j++){ind*=2;}
                for(int j = 0; j < ind; j++){strBuffer[ind-j-1] = ((*(int*)cfg[i].ptr >> j) & 0b1) +'0';} strBuffer[ind]='\0';
                fprintf(filehandle, "%s=%s;", cfg[i].name, strBuffer);
            }
            if(strlen(cfg[i].desc) > 0){fprintf(filehandle, " //%s" TXTNL, cfg[i].desc); //add comments
            } else {fprintf(filehandle, TXTNL);}
        }

        fprintf(filehandle, TXTNL "cfg_version=0x%X; //%s", (int)config_sum(cfg, cfg_size), "Config file version"); //write config version
        
        fclose(filehandle);
        printfTerm("%s: %s\n", reset ? strConfigTerm[STR_CONFIG_TERM::TERM_RESET_SUCCESS][language] : strConfigTerm[STR_CONFIG_TERM::TERM_SAVE_SUCCESS][language], filename);

        #if (!defined _WIN32 && !defined __CYGWIN__)
            if (uid !=-1 && gid != -1){ //config file owner
                struct stat file_stat = {0}; bool failed = false;
                if (stat(filename, &file_stat) == 0){
                    if (uid !=-1 && (file_stat.st_uid != uid || gid !=-1) && file_stat.st_gid != gid) {
                        if (chown(filename, (uid_t) uid, (gid_t) gid) < 0){failed = true;
                        } else {debug_stdout("%s owner changed successfully (uid:%d, gid:%d)\n", filename, uid, gid);}
                    }
                } else {failed = true;}
                if (failed){debug_stderr("changing %s owner failed with errno:%d (%m)\n", filename, errno); return -errno;}
            }
        #endif

        return 0;
    } else {
        printfTerm(strConfigTerm[STR_CONFIG_TERM::TERM_WRITE_FAILED][language], filename, errno);
        printf("\n");
    }
    return -errno;
}

int config_set(cfg_vars_t* cfg, unsigned int cfg_size, char* filename, int uid, int gid, bool readcfg, char* var_value){ //update var based on config file, var_value format: var=value
    if (readcfg){config_parse(cfg, cfg_size, filename, uid, gid, false);} //parse config file, create if needed
    int ret; char *tmpPtr = strchr(var_value, '='); //'=' char position
    if (tmpPtr != NULL){ //contain '='
        *tmpPtr='\0'; int tmpVarSize = strlen(var_value), tmpValSize = strlen(tmpPtr+1); //var and val sizes
        char tmpVar [tmpVarSize+1]; strcpy(tmpVar, var_value); //extract var
        char tmpVal [tmpValSize+1]; strcpy(tmpVal, tmpPtr + 1); //extract val
        int tmpIndex = config_search_name (cfg, cfg_size, tmpVar, true); //var in config array
        if (tmpIndex != -1) { //found in config array
            if (config_type_parse(cfg, cfg_size, tmpIndex, cfg[tmpIndex].type, tmpVar, tmpVal)){ //parse config var with specific type
                printfTerm(strConfigTerm[STR_CONFIG_TERM::TERM_VAR_UPD][language], tmpVar, tmpVal);
                printf("\n");
                return 0; //valid
            } else {
                printfTerm(strConfigTerm[STR_CONFIG_TERM::TERM_VAR_UPD_FAILED][language], tmpVar);
                printf("\n");
                ret = -EPERM;
            }
        } else {
            printfTerm(strConfigTerm[STR_CONFIG_TERM::TERM_VAR_UPD_MISSING][language], tmpVar);
            printf("\n");
            ret = -EPERM;
        }
    } else {debug_stderr("FATAL: Config: invalid var_value argument, format: VAR=VALUE\n"); ret = -EPERM;}
    return ret;
}

bool config_type_parse(cfg_vars_t* cfg, unsigned int cfg_size, unsigned int index, int type, char* var, char* value){ //parse config var with specific type
    if (index < 0 || index > cfg_size){debug_stderr("invalid index(%d), limit:0-%d\n", index, cfg_size); return false;}
    int tmpValSize = strlen(value);
    if (tmpValSize == 0 || strlen(var) == 0){
        debug_stderr("invalid var or value (empty)\n");
        return false;
    }
    debug_stderr("input:'%s'\n", value);
    if (type == 0){
        *(int*)cfg[index].ptr = atoi(value); //int
        debug_stderr("%s=%d (type:%d)\n", var, *(int*)cfg[index].ptr, type);
    } else if (type == 1){
        *(unsigned int*)cfg[index].ptr = atoi(value); //unsigned int
        debug_stderr("%s=%u (type:%d)\n", var, *(unsigned int*)cfg[index].ptr, type);
    } else if (type == 2){
        *(float*)cfg[index].ptr = atof(value); //float
        debug_stderr("%s=%f (type:%d)\n", var, *(float*)cfg[index].ptr, type);
    } else if (type == 3){
        *(double*)cfg[index].ptr = atof(value); //double
        debug_stderr("%s=%lf (type:%d)\n", var, *(double*)cfg[index].ptr, type);
    } else if (type == 4){
        *(bool*)cfg[index].ptr = (atoi(value) > 0)?true:false; //bool
        debug_stderr("%s=%d (type:%d)\n", var, (*(bool*)cfg[index].ptr) ? 1 : 0, type);
    } else if (type == 5){ //int array, input format: var=%d,%d,%d,...;
        int arrSize = *(int*)((int*)cfg[index].ptr)[1]; int j = 0;
        char tmpVal1 [tmpValSize+1]; strcpy(tmpVal1, value);
        char *tmpPtr2 = strtok(tmpVal1, ",");
        while (tmpPtr2 != NULL){
            if (j < arrSize) {((int*)((int*)cfg[index].ptr)[0])[j] = atoi(tmpPtr2);} //no overflow
            j++; tmpPtr2 = strtok(NULL, ","); //next element
        }
        if(debug){
            if (j != arrSize){debug_stderr("Warning var '%s' elements count mismatch, should have %d but has %d\n", var, arrSize, j);}
            char strBuffer1[4096]={'\0'}, strTmpBuffer[4096]; strTmpBuffer[0] = '\0'; int strBufferSize = 0;
            for (j = 0; j < arrSize; j++) {strBufferSize += sprintf (strBuffer1, "%d,", ((int*)((int*)cfg[index].ptr)[0])[j]); strcat (strTmpBuffer, strBuffer1);} *(strTmpBuffer+strBufferSize-1) = '\0';
            debug_stderr("%s=%s (type:%d)\n", var, strTmpBuffer, type);
        }
    } else if (type >= 6 && type < 9){ //hex8-32
        int ind = 2; for(int j=0; j<type-6; j++){ind*=2;}
        char *tmpPtr2 = strchr(value,'x');
        if(tmpPtr2!=NULL){
            char strTmpBuffer[12];
            sprintf(strTmpBuffer, "0x%%0%dX", ind);
            sscanf(value, strTmpBuffer, cfg[index].ptr);
        }else{
            debug_stderr("Warning var '%s' should have a hex value, assumed a int\n", var);
            *(int*)cfg[index].ptr = atoi(value);
        }
        debug_stderr("%s=0x%X(%d) (type:%d)\n", var, *(int*)cfg[index].ptr, *(int*)cfg[index].ptr, type);
    } else if (type >= 9 && type < 12){ //bin8-32
        int ind = 8; for(int j=0; j<type-9; j++){ind*=2;}
        if (debug && tmpValSize!=ind){debug_stderr("Warning var '%s' value lenght mismatch, needs %d but has %d\n", var, ind, tmpValSize);}
        
        int tmp = 0; for(int j = 0; j < ind; j++){if (j<tmpValSize) {if(value[tmpValSize-j-1] > 0+'0'){tmp ^= 1U << j;}}}
        *(int*)cfg[index].ptr = tmp;

        if(debug){
            char strTmpBuffer[34];
            for(int j = 0; j < ind; j++){strTmpBuffer[ind-j-1] = ((*(int*)cfg[index].ptr >> j) & 0b1) +'0';} strTmpBuffer[ind]='\0';
            debug_stderr("%s=%s(%d) (type:%d)\n", var, strTmpBuffer, *(int*)cfg[index].ptr, type);
        }
    } else {
        debug_stderr("invalid type:%d\n", type);
        return false;
    }

    return true;
}

bool config_value_to_str(char* str, unsigned int strSize, int type, void* value){ //convert void pointer to char array string using given cfg_vars_struct type
    if (strSize < 1 || str == NULL || value == NULL){return false;}
    unsigned int nChars = 0;
    if (type == 0){nChars = snprintf(str, strSize, "%d", *(int*)value); //int
    } else if (type == 1){nChars = snprintf(str, strSize, "%u", *(unsigned int*)value); //unsigned int
    } else if (type == 2){nChars = snprintf(str, strSize, "%f", *(float*)value); //float
    } else if (type == 3){nChars = snprintf(str, strSize, "%lf", *(double*)value); //double
    } else if (type == 4){nChars = snprintf(str, strSize, "%d", (*(bool*)value) ? 1 : 0); //bool
    } else if (type == 5){ //int array, output format: %d,%d,%d,...
        char strBuffer[4096], strBuffer1[33];
        unsigned int arrSize = *(int*)((int*)value)[1];
        for (unsigned int j = 0; j < arrSize; j++){
            nChars += sprintf(strBuffer1, "%d,", ((int*)((int*)value)[0])[j]);
            strcat(strBuffer, strBuffer1);
        }
        *(strBuffer + nChars - 1) = '\0';
        nChars = snprintf(str, strSize, "%s", strBuffer);
    } else if (type >= 6 && type < 9) { //hex8-32
        char strBuffer[8];
        int ind = 2; for(int j=0; j<type-6; j++){ind*=2;}
        sprintf(strBuffer, "0x%%0%dX", ind); //build that way to limit var size
        nChars = snprintf(str, strSize, strBuffer, *(int*)value);
    } else if (type >= 9 && type < 12) { //bin8-32 (itoa bypass)
        char strBuffer[34];
        int ind = 8; for(int j=0; j<type-9; j++){ind*=2;}
        for(int j = 0; j < ind; j++){
            strBuffer[ind-j-1] = ((*(int*)value >> j) & 0b1) +'0';
        }
        strBuffer[ind]='\0';
        nChars = snprintf(str, strSize, "%s", strBuffer);
    } else { //invalid type
        *str = '\0';
    }

    if (nChars == 0 || nChars > strSize){return false;}
    return true;
}

int config_parse(cfg_vars_t* cfg, unsigned int cfg_size, char* filename, int uid, int gid, bool createFile){ //parse/create program config file, return -errno on failure, 0 on success, 1 if new config created
    int ret = 0;
    FILE *filehandle = fopen(filename, "r");
    if (filehandle != NULL){
        char strBuffer[4096], strTmpBuffer[4096]; //string buffer
        char *tmpPtr, *tmpPtr1, *tmpPtr2, *pos; //pointers
        int cfg_ver=0, line=0;
        while (fgets(strBuffer, 4095, filehandle) != NULL){ //line loop
            line++; //current file line

            //clean line from utf8 bom and whitespaces
            tmpPtr = strBuffer; tmpPtr1 = strTmpBuffer; pos = tmpPtr;
            tmpPtr2 = strstr(strBuffer, "\xEF\xBB\xBF");
            if (tmpPtr2 != NULL){tmpPtr = tmpPtr2 + 3; debug_stderr("skipping UTF8 BOM\n");}

            while (*tmpPtr != '\0'){ //read all chars, copy if not whitespace
                if (tmpPtr - pos > 0){if(*tmpPtr=='/' && *(tmpPtr-1)=='/'){tmpPtr1--; break;}} //break if // comment
                if (!isspace(*tmpPtr)){*tmpPtr1++ = *tmpPtr++;} else {tmpPtr++;} //normal char, whitespace
            }
            *tmpPtr1='\0'; //copy cleaned line

            char* tokenPtr = strtok(strTmpBuffer, ";"); //split element
            while (tokenPtr != NULL){ //var=val; loop
                char* valPtr = strchr(tokenPtr, '='); //'=' char position
                if (valPtr != NULL){
                    char* varPtr = tokenPtr; //variable str pointer
                    *valPtr++ = '\0'; //value str pointer
                    int tmpIndex = config_search_name(cfg, cfg_size, varPtr, true); //var in config array
                    if (tmpIndex != -1){ //found in config array
                        config_type_parse(cfg, cfg_size, tmpIndex, cfg[tmpIndex].type, varPtr, valPtr);
                    } else if (strcmp(varPtr, "cfg_version") == 0){ //config version
                        sscanf(valPtr, "0x%X", &cfg_ver);
                        debug_stderr("cfg_version=0x%X (%d)\n", cfg_ver, cfg_ver);
                    } else { //invalid var
                        debug_stderr("var '%s'(line:%d) not allowed, typo?\n", varPtr, line);
                    }
                }
                tokenPtr = strtok(NULL, ";"); //next element
            }
        }
        fclose(filehandle);

        //pseudo checksum for config build
        int cfg_ver_org = config_sum(cfg, cfg_size); 
        if(cfg_ver != cfg_ver_org){
            debug_stderr("config file version mismatch (got:%d, should be:%d), forcing save to implement new vars set\n", cfg_ver, cfg_ver_org);
            ret = config_save(cfg, cfg_size, filename, uid, gid, false);
        }
    } else if (createFile){
        debug_stderr("config file not found, creating a new one\n");
        int tmp_ret = config_save(cfg, cfg_size, filename, uid, gid, false);
        ret = (tmp_ret == 0) ? 1 : tmp_ret;
    } else {ret = -1;} //missing config file
    return ret;
}

void config_list(cfg_vars_t* cfg, unsigned int cfg_size){ //print all config vars
    char *rowPtr;
    fprintf(stderr, "Valid config vars:\n");
    for (unsigned int i = 0; i < cfg_size; i++) {
        char tmpVar [strlen(cfg[i].name)+1];
        strcpy (tmpVar, cfg[i].name);
        if (tmpVar[0]=='\n') {rowPtr = tmpVar + 1;} else {rowPtr = tmpVar;}
        fprintf(stderr, "\t'%s': %s\n", rowPtr, cfg[i].desc);
    }
}

