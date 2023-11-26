/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to OpenGl window.

Important: If you update ImGui, please edit imgui_widgets.cpp and add in ImGui::BeginMainMenuBar() window_flags : " | ImGuiWindowFlags_NoBringToFrontOnFocus"
Note: This file is kind of a mess and may be reworked in the future.

upd 0.4a ok
*/

#ifndef GLCORE_H
#define GLCORE_H

#include <unistd.h>
#include <new>          // std::nothrow
#include <sys/time.h>
#include <limits.h>

extern "C" {
#include <png.h>
#include <zlib.h>
#include "gd.h"
}

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

#include "debug.h"
#include "config.h"
#include "ncparser.h"
#include "gdcore_depth.h"
#include <errno.h>

//deal with long on 32bits system
#if (LONG_MAX == INT_MAX)
    #define LONG_TYPE long long
    #define LONG_TYPE_FORMAT "%lld"
    #define ULONG_TYPE unsigned long long
    #define ULONG_TYPE_FORMAT "%llu"
#else
    #define LONG_TYPE long
    #define LONG_TYPE_FORMAT "%ld"
    #define ULONG_TYPE unsigned long
    #define ULONG_TYPE_FORMAT "%lu"
#endif

//functs
int strNumSepFormatingLength(LONG_TYPE num); //detect number length including 1000 char separator and terminating null char
void strNumSepFormating(char* str, int strLength, char separator, LONG_TYPE num); //format number with given 1000 char separator and terminating null char

void glCallbackRefresh(GLFWwindow* window); //force update viewport on context deterioration or window change

int generateShaderProgramFromFile(unsigned int* program, char* vertexShaderFile, char* fragmentShaderFile, bool debugOutput); //generate shader program from file
int generateShaderProgram(unsigned int* program, char* vertexShaderArr, char* fragmentShaderArr, bool debugOutput); //generate shader program
void glBuildVertexArray(unsigned int* vao, unsigned int* vbo, float* verticesArr, int verticesArrSize, int pointsPerVertice); //generate VAO and VBO arrays

void glUpdateCamPosition(double rotXdiff, double rotYdiff, double slideXdiff, double slideYdiff, double zoomYdiff, bool forceUpdate); //update camera position
void glUpdateDepth(float* bondaries, float* near, float* far); //update depth, for shader usage

int glGenerateArcVertices(float* array, unsigned int startIndex, double centerX, double centerY, double zStart, double zEnd, double xSize, double ySize, double startAngle, double endAngle, double resolution); //Generate vertices from arc data and add them to a given array, set array to NULL to avoid vertice generation and only return vertice count, only work normal to XZ

int glExportOBJ(char* file, float* verticesArray, int pointsPerVertice, int triangleCount, float offsetX, float offsetY, float offsetZ, float scaling); //export vertices array to obj file
int glExportSTLbin(char* file, float* verticesArray, int pointsPerVertice, int triangleCount, float offsetX, float offsetY, float offsetZ, float scaling); //export vertices array to binary stl file

void glProgramClose(void); //run when gl program closes
int glPreview(char* file, ncLineStruct* lines, ncToolStruct* tools, ncCommentStruct* comments, ncLimitStruct* limits, ncSummaryStruct* summary, ncArraySize* arrSizes, bool debugOutput);


//main
bool debugBack = false;

//pointers to allocated memory
bool glAlreadyKilled = false; //program already killed
float* verticesGridPtr = nullptr;
float* verticesSubGridPtr = nullptr;
float* gridBondariesPtr = nullptr;
float* verticesAxisXPtr = nullptr;
float* verticesAxisYPtr = nullptr;
float* verticesAxisZPtr = nullptr;
float* verticesLimitsPtr = nullptr;
float* verticesHeightMapPtr = nullptr;
float* layersVertArraysPtr[NCPARSER_COMMENT_ARRAY][4] = {nullptr};

//shaders, done that way for debug purpose
#include "glcore_shaders.h"
#define shaderTypeCount 3
#define shaderStrBufferSize 131072 //buffer size if vertex/fragment read from hdd
typedef struct shaderTypeStruct {unsigned int* program; int shaderType;} shaderType; //ptr to program, shaderType related to shaderPaths[] index
const char* shaderPaths[shaderTypeCount][2][2] = { //{{shaderFilepath, fallbackArray}, {fragmentFilepath, fallbackArray}}
    { //Lines
        {"glsl_shaders/Lines_vertexShader.glsl", vertexShaderLinesStr}, //vertex
        {"glsl_shaders/Lines_fragmentShader.glsl", fragmentShaderLinesStr}, //fragment
    },
    { //ToolLines
        {"glsl_shaders/ToolLines_vertexShader.glsl", vertexShaderToolLinesStr}, //vertex
        {"glsl_shaders/ToolLines_fragmentShader.glsl", fragmentShaderToolLinesStr}, //fragment
    },
    { //HeightMap
        {"glsl_shaders/HeightMap_vertexShader.glsl", vertexShaderHeightMapStr}, //vertex
        {"glsl_shaders/HeightMap_fragmentShader.glsl", fragmentShaderHeightMapStr}, //fragment
    },
};

//draw specific
//extern bool glViewportEnable;
extern unsigned int glViewportWidth, glViewportHeight;
extern float glDrawArcPrecision, glHeightMapPrecision;

//colors
const float glColGrid[] = {0.05f, 0.05f, 0.05f, 0.5f};
const float glColSubGrid[] = {0.1f, 0.1f, 0.1f, 0.3f};
const float glColAxisX[] = {1.0f, 0.0f, 0.0f, 0.5f};
const float glColAxisY[] = {0.0f, 1.0f, 0.0f, 0.5f};
const float glColAxisZ[] = {0.0f, 0.0f, 1.0f, 0.5f};
const float glColLimits[] = {1.0f, 0.5f, 0.0f, 0.25f};
const float glColToolHighlight[] = {1.0f, 1.0f, 1.0f, 1.0f};
const float glColHeightMapMin[] = {0.0f, 0.3f, 0.3f, 1.0f};
const float glColHeightMapMax[] = {0.0f, 0.8f, 0.8f, 1.0f};
const float glColHeightMapCrash[] = {0.85f, 0.60f, 0.15f, 1.0f};
const float glColTools[][4] = {
    {1.0f, 0.25f, 0.0f, 0.8f},
    {0.15f, 0.30f, 0.8f, 0.9f},
    {0.0f, 0.85f, 0.0f, 0.9f},
    {1.0f, 1.0f, 0.0f, 0.9f},
};

float glMouseScrollRatio = 0.1, glCamRotRatio = 0.005, glCamSlideRatio = 0.05, glDrawRatio = 0.05; //ratios

glm::mat4 glProjection = glm::mat4(1.0f); //world projection
glm::mat4 glCamView = glm::mat4(1.0f); //camera matrix
float glCamX = 0., glCamY = 0., glCamZ = 0.; //camera position
float glCamAtX = 0., glCamAtY = 0., glCamAtZ = 0.; //camera lootAt position
float glCamYaw = 0., glCamPitch = 0.; //camera rotation
float glCamAtDist = 5.; 

float glCamAtXinit = 0., glCamAtYinit = 0., glCamAtZinit = 0., glCamYawInit = 0., glCamPitchInit = 0., glCamSlideRatioInit = 0.; //initial values
bool glVsyncEnabled = true, glAAenabled = false, glWireframeEnabled = false, glViewportUpdate = true, glViewportForceUpdate = false, glImGuiFocused = false;


//ansicon.h
extern void printfTerm(const char* format, ...); //ansicon.h: printf clone to do UTF8 to CP850 on the fly, based on printf source code

//config.h
extern int config_save(cfg_vars_t* cfg, unsigned int cfg_size, char* filename, int uid, int gid, bool reset); //save config file
extern bool config_value_to_str(char* str, unsigned int strSize, int type, void* value); //convert void pointer to char array string using given cfg_vars_struct type

//nc2png.h
extern char programversion[]; //program version
extern unsigned int language; //language id
extern unsigned int speedPercent; //max running percent speed, used as a override
extern cfg_vars_t cfg_vars[]; //comments only accept one language
extern unsigned int cfg_vars_arr_size_tmp; //config array size
const unsigned int cfg_vars_arr_size = cfg_vars_arr_size_tmp; //bypass linker glitch
extern const char* cfg_filename_tmp; //path to config file
const char* cfg_filename = cfg_filename_tmp; //bypass linker glitch
extern unsigned int streamBufferSize; //try to optimize file writing using bigger buffer

//gdcore.h
extern void sec2charArr(char* arr, double time); //convert seconds to char array in format : XXsec / XXmin / XXh XXmin
extern int NNSclampInt(int num, int min, int max); //clamp int value
extern float NNSfloatFade(float low, float high, float fade); //proportional float gradian, fade:0.0 to 1.0
extern float NNSfloatFadeNoClamp(float low, float high, float fade); //proportional float gradian, fade:0.0 to 1.0, no clamping
extern void NNSgdImageFill(gdImagePtr image, int color); //gdImageFill() replacement to avoid transparent color glich




#endif