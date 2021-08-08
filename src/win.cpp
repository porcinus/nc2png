/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Windows specific.
*/

#include "win.h"

bool CtrlHandler (DWORD signal) { //handle ctrl-c on windows to avoid ansicon glitch that doesn't reset ansi code
    if (signal == CTRL_C_EVENT) {
        printf ("\033[0m");
        if(debug){fprintf(stderr,"DEBUG: Ctrl+C triggered\n");}
        exit (0);
    }
    return true;
}

void UTF8toCP850 (char* str, char* output) { //convert utf8 chars to CP850 for proper display
    // chars map arrays based on https://www.ascii-codes.com/cp850.html and https://www.utf8-chartable.de/
    const unsigned char convTableC2 [] = {255 /*NBSP*/, 173 /*¡*/, 189 /*¢*/, 156 /*£*/, 207 /*¤*/, 190 /*¥*/, 221 /*¦*/, 245 /*§*/, 249 /*¨*/, 184 /*©*/, 166 /*ª*/, 174 /*«*/, 170 /*¬*/, 240 /*­*/, 169 /*®*/, 238 /*¯*/, 248 /*°*/, 241 /*±*/, 253 /*²*/, 252 /*³*/, 239 /*´*/, 230 /*µ*/, 244 /*¶*/, 250 /*·*/, 247 /*¸*/, 251 /*¹*/, 167 /*º*/, 175 /*»*/, 172 /*¼*/, 171 /*½*/, 243 /*¾*/, 168 /*¿*/};
    const unsigned char convTableC3 [] = {183 /*À*/, 181 /*Á*/, 182 /*Â*/, 199 /*Ã*/, 142 /*Ä*/, 143 /*Å*/, 146 /*Æ*/, 128 /*Ç*/, 212 /*È*/, 144 /*É*/, 210 /*Ê*/, 211 /*Ë*/, 222 /*Ì*/, 214 /*Í*/, 215 /*Î*/, 216 /*Ï*/, 209 /*Ð*/, 165 /*Ñ*/, 227 /*Ò*/, 224 /*Ó*/, 226 /*Ô*/, 229 /*Õ*/, 153 /*Ö*/, 158 /*×*/, 157 /*Ø*/, 235 /*Ù*/, 233 /*Ú*/, 234 /*Û*/, 154 /*Ü*/, 237 /*Ý*/, 232 /*Þ*/, 225 /*ß*/, 133 /*à*/, 160 /*á*/, 131 /*â*/, 198 /*ã*/, 132 /*ä*/, 134 /*å*/, 145 /*æ*/, 135 /*ç*/, 138 /*è*/, 130 /*é*/, 136 /*ê*/, 137 /*ë*/, 141 /*ì*/, 161 /*í*/, 140 /*î*/, 139 /*ï*/, 208 /*ð*/, 164 /*ñ*/, 149 /*ò*/, 162 /*ó*/, 147 /*ô*/, 228 /*õ*/, 148 /*ö*/, 246 /*÷*/, 155 /*ø*/, 151 /*ù*/, 163 /*ú*/, 150 /*û*/, 129 /*ü*/, 236 /*ý*/, 231 /*þ*/, 152 /*ÿ*/};
    char *tmpPtr = str; char *tmpPtr1 = output;
    while (*tmpPtr != '\0') {
        if (*tmpPtr == '\xC2') {tmpPtr++; *tmpPtr1 = (char)convTableC2[(unsigned char)*tmpPtr - 160];
        } else if (*tmpPtr == '\xC3') {tmpPtr++; *tmpPtr1 = (char)convTableC3[(unsigned char)*tmpPtr - 128];
        } else {*tmpPtr1 = *tmpPtr;}
        tmpPtr++; tmpPtr1++;
    }
    *tmpPtr1 ='\0';
}




