/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to OpenGl window.

Important: If you update ImGui, please edit imgui_widgets.cpp and add in ImGui::BeginMainMenuBar() window_flags : " | ImGuiWindowFlags_NoBringToFrontOnFocus"
*/

#ifndef GL_H
#define GL_H

#include <unistd.h>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui/imconfig.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <new>          // std::nothrow
#include <sys/time.h>

#include "ncparser.h"

enum STR_GL {NC_NOCOMMENT,
LAYER_DISPLAY,LAYER_HIDE,LAYER_REPORT,LAYER_LAYERS,
TOOL_FAST,TOOL_WORK,TOOL_CIRC,TOOL_DRILL,TOOL_TOTAL,TOOL_ZCLIPPING,
MENU_TOOLPATH,MENU_DISPLAY,MENU_RESETVIEW,MENU_AXIS,MENU_GRID,MENU_SUBGRID,MENU_LIMITS,MENU_SETTINGS,MENU_AA,MENU_LIMITFPS,MENU_FORCEUPD,MENU_CFG,MENU_ABOUT,
CFG_APPLYWARN,
WINDOW_SAVE,WINDOW_RESETDEFAULT,WINDOW_CANCEL,ABOUT_EXTLIB,WINDOW_CLOSE};

const char *strGlEN [] = {
/*NC_NOCOMMENT*/ "Program",

/*LAYER_DISPLAY*/ "Show",
/*LAYER_HIDE*/ "Hide",
/*LAYER_REPORT*/ "Report",
/*LAYER_LAYERS*/ "Layers",

/*TOOL_FAST*/ "Fast",
/*TOOL_WORK*/ "Work",
/*TOOL_CIRC*/ "Circular",
/*TOOL_DRILL*/ "Drilling",
/*TOOL_TOTAL*/ "Total",
/*TOOL_ZCLIPPING*/ "Z clipping",

/*MENU_TOOLPATH*/ "Toolpaths",
/*MENU_DISPLAY*/ "Display",
/*MENU_RESETVIEW*/ "Reset viewport",
/*MENU_AXIS*/ "Axis",
/*MENU_GRID*/ "Grid",
/*MENU_SUBGRID*/ "Sub-grid",
/*MENU_LIMITS*/ "Limits",
/*MENU_SETTINGS*/ "Settings",
/*MENU_AA*/ "Anti-aliasing",
/*MENU_LIMITFPS*/ "Limit to 60fps",
/*MENU_FORCEUPD*/ "Force viewport update",
/*MENU_CFG*/ "Edit configuration",
/*MENU_ABOUT*/ "About...",

/*CFG_APPLYWARN*/ "Apply after program restart",

/*WINDOW_SAVE*/ "Save",
/*WINDOW_RESETDEFAULT*/ "Reset config to default",
/*WINDOW_CANCEL*/ "Cancel",
/*ABOUT_EXTLIB*/ "External libraries",
/*WINDOW_CLOSE*/ "Close"
};

const char *strGlFR [] = {
/*NC_NOCOMMENT*/ "Programme",

/*LAYER_DISPLAY*/ "Afficher",
/*LAYER_HIDE*/ "Masquer",
/*LAYER_REPORT*/ "Rapport",
/*LAYER_LAYERS*/ "Couches",

/*TOOL_FAST*/ "Rapide",
/*TOOL_WORK*/ "Travail",
/*TOOL_CIRC*/ "Circulaire",
/*TOOL_DRILL*/ "Perçage",
/*TOOL_TOTAL*/ "Total",
/*TOOL_ZCLIPPING*/ "Limite Z",

/*MENU_TOOLPATH*/ "Chemin d'outils",
/*MENU_DISPLAY*/ "Affichage",
/*MENU_RESETVIEW*/ "RàZ de la vue",
/*MENU_AXIS*/ "Axes",
/*MENU_GRID*/ "Grille",
/*MENU_SUBGRID*/ "Sous-grille",
/*MENU_LIMITS*/ "Limites",
/*MENU_SETTINGS*/ "Paramètres",
/*MENU_AA*/ "Anti-aliasing",
/*MENU_LIMITFPS*/ "Limiter à 60fps",
/*MENU_FORCEUPD*/ "Forcer la mise a jour",
/*MENU_CFG*/ "Editer la config",
/*MENU_ABOUT*/ "A propos de...",

/*CFG_APPLYWARN*/ "Appliqué après redémarrage du programme",

/*WINDOW_SAVE*/ "Sauvegarder",
/*WINDOW_RESETDEFAULT*/ "RàZ config",
/*WINDOW_CANCEL*/ "Annuler",
/*ABOUT_EXTLIB*/ "Librairies externe",
/*WINDOW_CLOSE*/ "Fermer"
};

const char *strGlCfgEN [] = {
"Maximum X and Y fast speed in mm/min",
"Maximum Z fast speed in mm/min",
"Speed override in percent",
"Preview grid max width/height in pixels",
"Preview arc resolution in pixels",
"Enable OpenGL preview, 0:No 1:Yes",
"OpenGL: Viewport width in pixels",
"OpenGL: Viewport height in pixels",
"OpenGL: Arc resolution in mm"
};

const char *strGlCfgFR [] = {
"Vitesse d'avance rapide des axes X et Y en mm/min",
"Vitesse d'avance rapide de l'axe Z en mm/min",
"Poucentage d'avance maximum",
"Taille de la grille maximum en pixels",
"Resolution maximum des arcs en pixels",
"Activer l'aperçu OpenGL, 0:Non 1:Oui",
"OpenGL: Largeur de la fenetre",
"OpenGL: Hauteur de la fenetre",
"OpenGL: Resolution maximum des arcs en mm"
};

const char **strGl [] = {strGlEN, strGlFR};
const char **strGlCfg [] = {strGlCfgEN, strGlCfgFR};


//functs
void glCallbackRefresh (GLFWwindow); //force update viewport on context deterioration or window change
void glUpdateDepth (float*, float*, float*); //update depth, for shader usage
void glBuildVertexArray (unsigned int*, unsigned int*, float*, int); //generate VAO and VBO arrays
void glUpdateCamPosition (double, double, double, double, double, bool); //update camera position
int generateShaderProgram (unsigned int*, const char*, const char*, bool); //generate shader program
int glGenerateArcVertices (float*, unsigned int, double, double, double, double, double, double, double, double, double); //Generate vertices from arc data and add them to a given array, set array to NULL to avoid vertice generation and only return vertice count, only work normal to XZ
int glPreview (ncLineStruc*, ncToolStruc*, ncDistTimeStruc*, ncLimitStruc*, ncLinesCountStruc*, ncArraySize*, bool);


//shaders
const char *vertexShaderLinesStr = "#version 330 core\n"
"layout (location = 0) in vec3 vertexPosition;"
"layout (std140) uniform viewportMats {mat4 projection; mat4 view;};"
"void main(){gl_Position = projection * view * vec4(vertexPosition, 1.0f);}\0";

const char *fragmentShaderLinesStr = "#version 330 core\n"
"uniform vec4 color = vec4 (1.f,1.f,1.f,1.f); out vec4 fragColor;"
"void main(){fragColor = color;}\0";

const char *vertexShaderToolLinesStr = "#version 330 core\n"
"layout (location = 0) in vec3 vertexPosition;"
"layout (std140) uniform viewportMats {mat4 projection; mat4 view;};"
"uniform bool clipPlaneYEn = false; uniform float clipPlaneY = 0.f;"
"uniform float far = 0.f; uniform float near = 0.f; uniform vec3 cam = vec3 (0.f,0.f,0.f);"
"out float depth; flat out int fragYclip;"
"void main(){"
"   gl_Position = projection * view * vec4(vertexPosition, 1.0f);"
"   if (clipPlaneYEn && vertexPosition.y > clipPlaneY) {fragYclip = 1; depth = 1.;"
"   } else {"
"       if ((abs (far) + abs (near)) > 0) {depth = clamp(1 - (distance (cam, vertexPosition) - near) * (1 / (far - near)) * 0.5f, 0.0001f, 1.f);} else {depth = 0.f;}"
"       fragYclip = 0;"
"   }"
"}\0";

const char *fragmentShaderToolLinesStr = "#version 330 core\n"
"uniform vec4 color = vec4 (1.f,1.f,1.f,1.f);"
"flat in int fragYclip;"
"in float depth;"
"out vec4 fragColor;"
"void main(){"
"   if (fragYclip > 0) {discard;} vec4 tmpColor = color;"
"   if (depth > 0.f) {tmpColor.xyz *= depth;} fragColor = tmpColor;"
"}\0";

//draw specific
unsigned int glViewportEnable = 1;
unsigned int glViewportWidth = 800, glViewportHeight = 600;
float glDrawArcPrecision = 1.0;

//colors
const float glColGrid [] = {0.05f,0.05f,0.05f,0.5f};
const float glColSubGrid [] = {0.1f,0.1f,0.1f,0.3f};
const float glColAxisX [] = {1.0f,0.0f,0.0f,0.5f};
const float glColAxisY [] = {0.0f,1.0f,0.0f,0.5f};
const float glColAxisZ [] = {0.0f,0.0f,1.0f,0.5f};
const float glColLimits [] = {1.0f,0.5f,0.0f,0.25f};
const float glColToolHighlight [] = {1.0f,1.0f,1.0f,1.0f};
const float glColTools [][4] = {{1.0f,0.25f,0.0f,0.8f},{0.15f,0.30f,0.8f,0.9f},{0.0f,0.85f,0.0f,0.9f},{1.0f,1.0f,0.0f,0.9f}};

float glMouseScrollRatio = 0.1, glCamRotRatio = 0.005, glCamSlideRatio = 0.05, glDrawRatio = 0.05; //ratios

glm::mat4 glCamView = glm::mat4(1.0f); //camera matrix
float glCamX = 0., glCamY = 0., glCamZ = 0.; //camera position
float glCamAtX = 0., glCamAtY = 0., glCamAtZ = 0.; //camera lootAt position
float glCamYaw = 0., glCamPitch = 0.; //camera rotation
float glCamAtDist = 5.; 

float glCamAtXinit = 0., glCamAtYinit = 0., glCamAtZinit = 0., glCamYawInit = 0., glCamPitchInit = 0., glCamSlideRatioInit = 0.; //initial values
bool glVsyncEnabled = true, glAAenabled = false, glViewportUpdate = true, glViewportForceUpdate = false, glImGuiFocused = false;



//config.h
extern bool configSave (bool, bool=false); //save config file
extern char *cfgVarsName[]; //config vars names
extern int cfgVarsType[]; //types : 0:int, 1:uint, 2:float, 3:double
extern void *cfgVarsPtr[]; //pointers to corresponding vars
extern unsigned int cfgVarsArrSize; //config pointers array size

//nc2png.h
extern char programversion[]; //program version
extern char libGDver[], libPNGver[], zlibver[];
extern bool debug; //debug mode bool
extern unsigned int speedFastXY; //max XY moving speed
extern unsigned int speedFastZ;  //max Z moving speed
extern unsigned int speedPercent; //max running percent speed, used as a override
extern unsigned int gdWidth; //max grid width/height in px
extern unsigned int gdArcRes; //arc drawing resolution
extern unsigned int language; //language id

//gdcore.h
extern void sec2charArr (char*, double); //convert seconds to char array in format : XXsec / XXmin / XXh XXmin




#endif