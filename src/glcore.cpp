/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to OpenGl window.

Important: If you update ImGui, please edit imgui_widgets.cpp and add in ImGui::BeginMainMenuBar() window_flags : " | ImGuiWindowFlags_NoBringToFrontOnFocus"
*/

#include "glcore.h"

void glCallbackRefresh (GLFWwindow* window) {glViewportUpdate = true;} //force update viewport on context deterioration or window change

int generateShaderProgram (unsigned int* program, const char* vertexShaderArr, const char* fragmentShaderArr, bool debugOutput) { //generate shader program
    int  glCompileSuccess = 0;
    char glLog[512]; 

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); //create shader
    glShaderSource(vertexShader, 1, &vertexShaderArr, NULL); glCompileShader(vertexShader); //attach and compile
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &glCompileSuccess); //get shader compilation status
    if (!glCompileSuccess){ //compile failed
        if(debugOutput){glGetShaderInfoLog(vertexShader, 512, NULL, glLog); printf ("Vertex shader failed to compile : %s\n", glLog);}
        return -1;
    } else {if(debugOutput){printf ("Vertex shader compile success\n");}}

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //create shader
    glShaderSource(fragmentShader, 1, &fragmentShaderArr, NULL); glCompileShader(fragmentShader); //attach and compile
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &glCompileSuccess); //get shader compilation status
    if (!glCompileSuccess){ //compile failed
        if(debugOutput){glGetShaderInfoLog(fragmentShader, 512, NULL, glLog); printf ("Fragment shader failed to compile : %s\n", glLog);}
        return -2;
    } else {if(debugOutput){printf ("Fragment shader compile success\n");}}

    *program = glCreateProgram(); glAttachShader(*program, vertexShader); glAttachShader(*program, fragmentShader); //create shader program and attach shader
    glLinkProgram(*program); //link shader program
    glGetProgramiv(*program, GL_LINK_STATUS, &glCompileSuccess); //get link status
    if (!glCompileSuccess) { //link failed
        if(debugOutput){glGetProgramInfoLog(*program, 512, NULL, glLog); printf ("Failed to create shader program : %s\n", glLog);}
        return -3;
    } else {if(debugOutput){printf ("Shader program create success\n");}}

    glDeleteShader(vertexShader); glDeleteShader(fragmentShader); //delete shaders
    return 0; //success
}

void glBuildVertexArray (unsigned int* vao, unsigned int* vbo, float* verticesArr, int arrSize) { //generate VAO and VBO arrays
    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, arrSize * sizeof(float), verticesArr, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glUpdateCamPosition (double rotXdiff, double rotYdiff, double slideXdiff, double slideYdiff, double zoomYdiff, bool forceUpdate) { //update camera position
    if (abs(slideXdiff) > 0.00001) { //update view slide x
        float tmpSlideAngleX = glCamYaw + 1.570795;
        float tmpSin = sin(tmpSlideAngleX) * slideXdiff * glCamSlideRatio, tmpCos = cos(tmpSlideAngleX) * slideXdiff * glCamSlideRatio;
        glCamAtX = glCamAtX + tmpSin; glCamAtZ = glCamAtZ + tmpCos;
        forceUpdate = true; //force update cam
    }
    
    if (abs(slideYdiff) > 0.00001) { //update view slide y
        float tmpSlideAngleY = glCamPitch + 1.570795;
        float tmpSinX = sin(glCamYaw), tmpCosX = cos(glCamYaw), tmpSinY = sin(tmpSlideAngleY), tmpCosY = cos(tmpSlideAngleY);
        glCamAtX = glCamAtX + tmpSinX * tmpCosY * slideYdiff * glCamSlideRatio;
        glCamAtZ = glCamAtZ + tmpCosX * tmpCosY * slideYdiff * glCamSlideRatio;
        glCamAtY = glCamAtY + tmpSinY * slideYdiff * glCamSlideRatio;
        forceUpdate = true; //force update cam
    }

    if (abs(rotXdiff) > 0.00001 || abs(rotYdiff) > 0.00001 || abs(zoomYdiff) > 0.00001 || forceUpdate) { //update view rotation or zoom
        float tmpGLCamX, tmpGLCamZ, tmpGLCamY;
        glCamYaw = glCamYaw - rotXdiff * glCamRotRatio;
        if (glCamYaw < -3.14159F) {glCamYaw = 3.14158F;} if (glCamYaw > 3.14159F) {glCamYaw = -3.14158F;} //yaw rollover
        tmpGLCamX = sin(glCamYaw); tmpGLCamZ = cos(glCamYaw);

        glCamPitch = glCamPitch + rotYdiff * glCamRotRatio;
        if (glCamPitch < -1.57079F) {glCamPitch = -1.57078F;} if (glCamPitch > 1.57079F) {glCamPitch = 1.57078F;} //limit pitch
        tmpGLCamY = sin(glCamPitch);
        float tmpcos = cos(glCamPitch);

        if (abs(zoomYdiff) > 0.00001) {
            glCamAtX = glCamAtX + tmpGLCamX * tmpcos * zoomYdiff;
            glCamAtZ = glCamAtZ + tmpGLCamZ * tmpcos * zoomYdiff;
            glCamAtY = glCamAtY + tmpGLCamY * zoomYdiff;
        }

        glCamX = glCamAtX + tmpGLCamX * tmpcos * glCamAtDist;
        glCamZ = glCamAtZ + tmpGLCamZ * tmpcos * glCamAtDist;
        glCamY = glCamAtY + tmpGLCamY * glCamAtDist;
        glViewportUpdate = true; //force update viewport
    }
}

void glUpdateDepth (float* bondaries, float* near, float* far) { //update depth, for shader usage
    float tmpDist = 0.f, tmpNear = 999.f, tmpFar = -999.f;
    glm::vec3 tmpCam = {glCamX, glCamY, glCamZ};
    for (unsigned int i = 0; i < 8; i+=2) {
        glm::vec3 tmpPos = {bondaries[i], 0.f, bondaries[i+1]};
        tmpDist = glm::distance (tmpPos, tmpCam);
        if (tmpDist > tmpFar) {tmpFar = tmpDist;} if (tmpDist < tmpNear) {tmpNear = tmpDist;}
    }
    *near = tmpNear; *far = tmpFar;
}

int glGenerateArcVertices (float* array, unsigned int startIndex, double centerX, double centerY, double zStart, double zEnd, double xSize, double ySize, double startAngle, double endAngle, double resolution) { //Generate vertices from arc data and add them to a given array, set array to NULL to avoid vertice generation and only return vertice count, only work normal to XZ
    if (numDiffDouble(startAngle, endAngle)<0.000001 || xSize < 0.000001 || ySize < 0.000001) {return 0;} if (resolution < 0.000001) {resolution = 0.000001;}
    double a1 = deg2rad(startAngle), a2 = deg2rad(endAngle), arcLenght = (abs(a2 - a1) * (xSize / 2) + abs(a2 - a1) * (ySize / 2)) / 2, hWidth = xSize/2, hHeight = ySize/2, steps = (double)ceil(arcLenght / resolution) + 1, stepangle = (a2 - a1) / steps;
    double lastX = centerX + cos(a1) * hWidth, lastY = centerY + sin(a1) * hHeight, x, y, z, lastZ = zStart; int createdVertice = 0;
    for (int i=1; i < steps + 1; i++) {
        if (array != NULL){
            x = centerX + cos(a1 + stepangle * i) * hWidth; y = centerY + sin(a1 + stepangle * i) * hHeight; z = zStart + ((zEnd - zStart) / steps) * i;
            array [startIndex + createdVertice + 0] = (float)lastX; array [startIndex + createdVertice + 1] = (float)lastZ; array [startIndex + createdVertice + 2] = (float)lastY;
            array [startIndex + createdVertice + 3] = (float)x; array [startIndex + createdVertice + 4] = (float)z; array [startIndex + createdVertice + 5] = (float)y;
            lastX = x; lastY = y; lastZ = z;
        }
        createdVertice += 6;
    }
    return createdVertice;
}

int glPreview (ncLineStruc *lines, ncToolStruc *tools, ncDistTimeStruc *ops, ncLimitStruc *limits, ncLinesCountStruc *linescount, ncArraySize *arrSizes, bool debugOutput) {
    GLFWwindow* window; //glfw window handle
    char windowTitle [] = "NC2PNG v00.00.00x (00000fps)"; sprintf (windowTitle, "NC2PNG %s", programversion); //window title
    int screenWidthLast = 0, screenHeightLast = 0, screenXoff = 0, screenWidth = 1, screenHeight = 1; //viewport size and offset

    struct timeval tv; gettimeofday(&tv, NULL); //timeval, used for pseudo Vsync and fps count
    double frameSleep; int glFPS = 0; unsigned long frameEndTV, frameStartTV, glFPSlastTime, glViewportLastUpdate;
    frameEndTV = frameStartTV = glFPSlastTime = glViewportLastUpdate = 1000000 * tv.tv_sec + tv.tv_usec; //init time

    if (!glfwInit()) {if(debugOutput){printf ("Failed to init GLFW\n");} return 1;} else {if(debugOutput){printf ("GLFW init success\n");}}
    
    window = glfwCreateWindow(glViewportWidth, glViewportHeight, windowTitle, NULL, NULL); //create gl window
    if (!window) {if(debugOutput){printf ("Failed to create GLFW window\n");} glfwTerminate(); return 1;} else {if(debugOutput){printf ("GLFW window created\n");}}

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwMakeContextCurrent(window); //make the window's context current
    glfwSwapInterval(0); //disable gdfw vsync

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {if(debugOutput){printf ("Failed to initialize OpenGL context\n");} glfwTerminate(); return 1;} else {if(debugOutput){printf ("OpenGL context init success\n");}}

    glEnable(GL_DEPTH_TEST); //enable z buffer
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (glAAenabled) {glEnable(GL_LINE_SMOOTH);} //smooth lines

    //ImGui init
    ImGui::CreateContext(); ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true); ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiWindowFlags sideBarFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
    ImGuiWindowFlags sliderBarFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration;
    ImGuiWindowFlags configWindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoNavFocus;
    ImGuiIO& ImGuiCurrentIO = ImGui::GetIO(); ImVec2 ImGuiMouseDelta; float ImGuiMouseWheel;
    ImGuiCurrentIO.IniFilename = NULL; //disable ini load/save

    //declare glfw callbacks
    glfwSetWindowRefreshCallback (window, &glCallbackRefresh); //window refresh callback

    //build XY (gcode) XZ (gl) limits
    int tmpXmin = floor(limits->xMin - 11.), tmpXmax = ceil (limits->xMax + 11.);
    int tmpYmin = floor(limits->yMin - 11.), tmpYmax = ceil (limits->yMax + 11.);
    int tmpZmin = floor(limits->zMin - 11.), tmpZmax = ceil (limits->zMax + 11.);
    if (tmpXmin > -11) {tmpXmin = -11;} if (tmpXmax < 11) {tmpXmax = 11;}
    if (tmpYmin > -11) {tmpYmin = -11;} if (tmpYmax < 11) {tmpYmax = 11;}
    if (tmpZmin > -11) {tmpZmin = -11;} if (tmpZmax < 11) {tmpZmax = 11;}

    //update camera lookAt vars
    glCamAtX = ((tmpXmin + tmpXmax) / 2) * glDrawRatio; glCamAtXinit = glCamAtX;
    glCamAtZ = ((tmpYmin + tmpYmax) / 2) * -glDrawRatio; glCamAtZinit = glCamAtZ;
    glCamAtY = ((tmpZmin + tmpZmax) / 2) * glDrawRatio; glCamAtYinit = glCamAtY;

    //generate grid subgrid vertices array, VBA, VBO
    unsigned int verticesGridIndex = 0, verticesSubGridIndex = 0, verticesCount = 0, VBOgrid, VAOgrid, VBOsubGrid, VAOsubGrid;
    unsigned int verticesGridCount = (ceil((double)(tmpXmax - tmpXmin) / 10.) + 1) * 6 + (ceil((double)(tmpYmax - tmpYmin) / 10.) + 1) * 6; //add 2 vertices per axis to avoid crash linked to poor rounding
    unsigned int verticesSubGridCount = (tmpXmax - tmpXmin + 2) * 6 + (tmpYmax - tmpYmin + 2) * 6 - verticesGridCount; //add 2 vertices per axis to avoid crash linked to poor rounding
    verticesCount += verticesGridCount + verticesSubGridCount;
    float *verticesGrid = new float [verticesGridCount](); float *verticesSubGrid = new float [verticesSubGridCount](); //vertices arrays, init to 0
    float *tmpPtrFloat; unsigned int *tmpPtrUInt; //pointers
    float tmpFloat0, tmpFloat1 = (float)-tmpYmax * glDrawRatio, tmpFloat2 = (float)-tmpYmin * glDrawRatio; //start end z
    for(int i = tmpXmin; i < tmpXmax; i++) {
        tmpFloat0 = (float) i * glDrawRatio; //start end x
        if (i%10 == 0) {tmpPtrUInt = &verticesGridIndex; tmpPtrFloat = verticesGrid + verticesGridIndex; //grid
        } else {tmpPtrUInt = &verticesSubGridIndex; tmpPtrFloat = verticesSubGrid + verticesSubGridIndex;} //subgrid
        *tmpPtrFloat = *(tmpPtrFloat + 3) = tmpFloat0; //start end x
        *(tmpPtrFloat + 2) = tmpFloat1; *(tmpPtrFloat + 5) = tmpFloat2; //start end z
        *tmpPtrUInt += 6; //increment vertice count
    }
    tmpFloat0 = (float)tmpXmin * glDrawRatio; tmpFloat2 = (float)tmpXmax * glDrawRatio; //start end  x
    for(int i = tmpYmin; i < tmpYmax; i++) {
        tmpFloat1 = (float) -i * glDrawRatio; //start end z
        if (i%10 == 0) {tmpPtrUInt = &verticesGridIndex; tmpPtrFloat = verticesGrid + verticesGridIndex; //grid
        } else {tmpPtrUInt = &verticesSubGridIndex; tmpPtrFloat = verticesSubGrid + verticesSubGridIndex;} //subgrid
        *(tmpPtrFloat) = tmpFloat0; *(tmpPtrFloat + 3) = tmpFloat2; //start end x
        *(tmpPtrFloat + 2) = tmpFloat1 = *(tmpPtrFloat + 5) = tmpFloat1; //start end z
        *tmpPtrUInt += 6; //increment vertice count
    }
    glBuildVertexArray (&VAOgrid, &VBOgrid, verticesGrid, verticesGridCount);
    glBuildVertexArray (&VAOsubGrid, &VBOsubGrid, verticesSubGrid, verticesSubGridCount);

    //grid bondaries for depth shader
    float *gridBondaries = new float [8]{
        (float) limits->xMin * glDrawRatio, (float) limits->yMin * -glDrawRatio, //xmin, zmin
        (float) limits->xMin * glDrawRatio, (float) limits->yMax * -glDrawRatio, //xmin, zmax
        (float) limits->xMax * glDrawRatio, (float) limits->yMin * -glDrawRatio, //xmax, zmin
        (float) limits->xMax * glDrawRatio, (float) limits->yMax * -glDrawRatio //xmax, zmax
    };
    float depthNear = 0., depthFar = 0.;

    //Y axis toolpath clipping
    float clippingYpos = (float)limits->zMax;
    char clippingYstr [strlen(strGl[language][STR_GL::TOOL_ZCLIPPING]) + 14]; sprintf (clippingYstr, "%s: %.2lfmm", strGl[language][STR_GL::TOOL_ZCLIPPING], clippingYpos);

    //generate axis array, VBA, VBO
    unsigned int verticesAxisCount = 24, VBOaxisX, VAOaxisX, VBOaxisY, VAOaxisY, VBOaxisZ, VAOaxisZ;
    float *verticesAxisX = new float [verticesAxisCount]{
        (float) tmpXmin * glDrawRatio, 0.0f, 0.0f, (float) tmpXmax * glDrawRatio, 0.0f, 0.0f, //start end x
        (float) (tmpXmax - 4.5) * glDrawRatio, 0.0f, 2 * glDrawRatio, (float) (tmpXmax - 4.5) * glDrawRatio, 0.0f, -2 * glDrawRatio, //arrow z+ z-
        (float) tmpXmax * glDrawRatio, 0.0f, 0.0f, //end x
        (float) (tmpXmax - 4.5) * glDrawRatio, 2 * glDrawRatio, 0.0f, (float) (tmpXmax - 4.5) * glDrawRatio, -2 * glDrawRatio, 0.0f, //arrow y+ y-
        (float) tmpXmax * glDrawRatio, 0.0f, 0.0f //end x
    };
    float *verticesAxisY = new float [verticesAxisCount]{
        0.0f, 0.0f, (float) tmpYmin * -glDrawRatio, 0.0f, 0.0f, (float) tmpYmax * -glDrawRatio, //start end y
        2 * glDrawRatio, 0.0f, (float) (tmpYmax - 4.5) * -glDrawRatio, -2 * glDrawRatio, 0.0f, (float) (tmpYmax - 4.5) * -glDrawRatio, //arrow x+ x-
        0.0f, 0.0f, (float) tmpYmax * -glDrawRatio, //end y
        0.0f, 2 * glDrawRatio, (float) (tmpYmax - 4.5) * -glDrawRatio, 0.0f, -2 * glDrawRatio, (float) (tmpYmax - 4.5) * -glDrawRatio, //arrow z+ z-
        0.0f, 0.0f, (float) tmpYmax * -glDrawRatio //end y
    };
    float *verticesAxisZ = new float [verticesAxisCount]{
        0.0f, (float) tmpZmin * glDrawRatio, 0.0f, 0.0f, (float) tmpZmax * glDrawRatio, 0.0f, //start end z
        2 * glDrawRatio, (float) (tmpZmax - 4.5) * glDrawRatio, 0.0f, -2 * glDrawRatio, (float) (tmpZmax - 4.5) * glDrawRatio, 0.0f, //arrow x+ x-
        0.0f, (float) tmpZmax * glDrawRatio, 0.0f, //end z
        0.0f, (float) (tmpZmax - 4.5) * glDrawRatio, 2 * glDrawRatio, 0.0f, (float) (tmpZmax - 4.5) * glDrawRatio, -2 * glDrawRatio, //arrow y+ y-
        0.0f, (float) tmpZmax * glDrawRatio, 0.0f //end z
    };
    verticesCount += verticesAxisCount * 3;
    glBuildVertexArray (&VAOaxisX, &VBOaxisX, verticesAxisX, verticesAxisCount);
    glBuildVertexArray (&VAOaxisY, &VBOaxisY, verticesAxisY, verticesAxisCount);
    glBuildVertexArray (&VAOaxisZ, &VBOaxisZ, verticesAxisZ, verticesAxisCount);

    //generate limits array, VBA, VBO
    unsigned int verticesLimitsCount = 24, VBOlimits, VAOlimits;
    float *verticesLimits = new float [verticesLimitsCount]{
        (float) limits->xMin * glDrawRatio, 0.f, tmpYmin * -glDrawRatio, (float) limits->xMin * glDrawRatio, 0.f, tmpYmax * -glDrawRatio, //xmin
        (float) limits->xMax * glDrawRatio, 0.f, tmpYmin * -glDrawRatio, (float) limits->xMax * glDrawRatio, 0.f, tmpYmax * -glDrawRatio, //xmax
        tmpXmin * glDrawRatio, 0.f, (float) limits->yMin * -glDrawRatio, tmpXmax * glDrawRatio, 0.f, (float) limits->yMin * -glDrawRatio, //ymin
        tmpXmin * glDrawRatio, 0.f, (float) limits->yMax * -glDrawRatio, tmpXmax * glDrawRatio, 0.f, (float) limits->yMax * -glDrawRatio //ymax
    };
    verticesCount += verticesLimitsCount;
    glBuildVertexArray (&VAOlimits, &VBOlimits, verticesLimits, verticesLimitsCount);

    //generate toolpath array, VBA, VBO
    unsigned int layersLimit = arrSizes->distTimeStrucLimit; //valid comment detected
    unsigned int layersVertCount [layersLimit][4] = {0}, layersVertIndex [layersLimit][4] = {0}, layersEnabled = 0; //vertices count/index per layer: g0, g1, g2-3, g81-83
    unsigned int layersVAO [layersLimit][4], layersVBO [layersLimit][4]; //layers vba, vbo: g0, g1, g2-3, g81-83
    bool layersEnable [layersLimit+1], layersHovered [layersLimit+1] = {false}, layersOpEnable [4] = {true,true,true,true};
    int g=-1, gPrev=-1, comment=0, tmpInt=0; unsigned int line=0, tmpUInt=0; float Zinit=.0f, tmpFloat3=.0f, tmpFloat4=.0f, tmpFloat5=.0f; //temp vars
    while (lines[line].g != -1 && line < linescount->all) { //pass to detect proper layers vertices count
        if (line > 0){ //avoid first line
            g = lines[line].g; gPrev = lines[line-1].g; comment = lines[line].comment;
            if (g == 0 && gPrev < 80) {layersVertCount[comment][0] += 6;
            } else if (g == 1) {layersVertCount[comment][1] += 6;
            } else if (g == 2 || g == 3) {
                if (numDiffDouble (lines[line].startAngle, lines[line].endAngle) > 1.) {
                    tmpInt = glGenerateArcVertices (NULL, 0, lines[line].i * glDrawRatio, lines[line].j * -glDrawRatio, lines[line-1].z * glDrawRatio, lines[line].z * glDrawRatio, lines[line].radius * glDrawRatio * 2, lines[line].radius * glDrawRatio * 2, lines[line].startAngle, lines[line].endAngle, glDrawArcPrecision * glDrawRatio);
                    if (tmpInt > 0) {layersVertCount[comment][2] += tmpInt;}
                }
                if (numDiffDouble (lines[line].startAngle1, lines[line].endAngle1) > 1.) {
                    tmpInt = glGenerateArcVertices (NULL, 0, lines[line].i * glDrawRatio, lines[line].j * -glDrawRatio, lines[line-1].z * glDrawRatio, lines[line].z * glDrawRatio, lines[line].radius * glDrawRatio * 2, lines[line].radius * glDrawRatio * 2, lines[line].startAngle1, lines[line].endAngle1, glDrawArcPrecision * glDrawRatio);
                    if (tmpInt > 0) {layersVertCount[comment][2] += tmpInt;}
                }
            } else if (g == 81 || g == 82 || g == 83) {layersVertCount[comment][3] += 6; if (gPrev == 81 || gPrev == 82 || gPrev == 83) {layersVertCount[comment][1] += 6;}}
        } line++;
    }

    //create vertices arrays
    float* layersVertArraysPrt [layersLimit][4]; //array pointers to all layers/kind
    double layerTotalDist [4] = {0.,0.,0.,0.}, layerTotalTime [4] = {0.,0.,0.,0.}; //store total dist/time

    if(debugOutput){printf ("layersLimit:%d\n",layersLimit);}
    for (unsigned int i=0; i<layersLimit; i++) { //nothrow used for debug
        if(debugOutput){printf ("layersVertArraysPrt:%d : [0]:%d, [1]:%d, [2]:%d, [3]:%d\n",i,layersVertCount[i][0],layersVertCount[i][1],layersVertCount[i][2],layersVertCount[i][3]);}
        layersVertArraysPrt[i][0] = new (std::nothrow) float [layersVertCount[i][0]](); if (!layersVertArraysPrt[i][0]) {printf("Failed to allocate layersVertArraysPrt[%d][0]\n", i); glfwTerminate(); return 1;}
        layersVertArraysPrt[i][1] = new (std::nothrow) float [layersVertCount[i][1]](); if (!layersVertArraysPrt[i][1]) {printf("Failed to allocate layersVertArraysPrt[%d][1]\n", i); glfwTerminate(); return 1;}
        layersVertArraysPrt[i][2] = new (std::nothrow) float [layersVertCount[i][2]](); if (!layersVertArraysPrt[i][2]) {printf("Failed to allocate layersVertArraysPrt[%d][2]\n", i); glfwTerminate(); return 1;}
        layersVertArraysPrt[i][3] = new (std::nothrow) float [layersVertCount[i][3]](); if (!layersVertArraysPrt[i][3]) {printf("Failed to allocate layersVertArraysPrt[%d][3]\n", i); glfwTerminate(); return 1;}
        verticesCount += layersVertCount[i][0] + layersVertCount[i][1] + layersVertCount[i][2] + layersVertCount[i][3];
        layersEnable [i] = true; layersHovered [i] = false; //default layer bools
        double tmpDoubleArr[] = {ops[i].distFast, ops[i].distWork, ops[i].distCircular, ops[i].distDrill}; //current ops dist
        double tmpDoubleArr1[] = {ops[i].timeFast, ops[i].timeWork, ops[i].timeCircular, ops[i].timeDrill}; //current ops time
        for (unsigned int j=0; j<4; j++) {layerTotalDist [j] += tmpDoubleArr[j]; layerTotalTime [j] += tmpDoubleArr1[j];} //add ops data for total
    }
    layersEnable [layersLimit] = true; //total line enable
    if(debugOutput){printf ("verticesCount:%d\n",verticesCount);}

    //generate vertices data
    line = 0;
    while (lines[line].g != -1 && line < linescount->all) {
        if (line > 0){
            g = lines[line].g; gPrev = lines[line-1].g; comment = lines[line].comment;
            tmpFloat0 = (float)lines[line-1].x * glDrawRatio; tmpFloat1 = (float)lines[line-1].z * glDrawRatio; tmpFloat2 = (float)lines[line-1].y * glDrawRatio * -1;
            tmpFloat3 = (float)lines[line].x * glDrawRatio; tmpFloat4 = (float)lines[line].z * glDrawRatio; tmpFloat5 = (float)lines[line].y * glDrawRatio * -1;
            if ((g == 0 || g == 1) && gPrev < 80) {
                tmpUInt = layersVertIndex[comment][g]; layersVertIndex[comment][g] += 6;
                layersVertArraysPrt[comment][g][tmpUInt] = tmpFloat0; layersVertArraysPrt[comment][g][tmpUInt+1] = tmpFloat1; layersVertArraysPrt[comment][g][tmpUInt+2] = tmpFloat2;
                layersVertArraysPrt[comment][g][tmpUInt+3] = tmpFloat3; layersVertArraysPrt[comment][g][tmpUInt+4] = tmpFloat4; layersVertArraysPrt[comment][g][tmpUInt+5] = tmpFloat5;
            } else if (g == 2 || g == 3) {
                tmpUInt = layersVertIndex[comment][2];
                if (numDiffDouble (lines[line].startAngle, lines[line].endAngle) > 1.) {
                    tmpInt = glGenerateArcVertices (layersVertArraysPrt[comment][2], layersVertIndex[comment][2], lines[line].i * glDrawRatio, lines[line].j * -glDrawRatio, lines[line-1].z * glDrawRatio, lines[line].z * glDrawRatio, lines[line].radius * glDrawRatio * 2, lines[line].radius * glDrawRatio * 2, lines[line].startAngle, lines[line].endAngle, glDrawArcPrecision * glDrawRatio);
                    if (tmpInt > 0) {layersVertIndex[comment][2] += tmpInt;}
                }
                if (numDiffDouble (lines[line].startAngle1, lines[line].endAngle1) > 1.) {
                    tmpInt = glGenerateArcVertices (layersVertArraysPrt[comment][2], layersVertIndex[comment][2], lines[line].i * glDrawRatio, lines[line].j * -glDrawRatio, lines[line-1].z * glDrawRatio, lines[line].z * glDrawRatio, lines[line].radius * glDrawRatio * 2, lines[line].radius * glDrawRatio * 2, lines[line].startAngle1, lines[line].endAngle1, glDrawArcPrecision * glDrawRatio);
                    if (tmpInt > 0) {layersVertIndex[comment][2] += tmpInt;}
                }
            } else if (g == 81 || g == 82 || g == 83) {
                tmpUInt = layersVertIndex[comment][3]; layersVertIndex[comment][3] += 6;
                if (lines[line].retractMode == 1){Zinit = (float)lines[line].startAngle * glDrawRatio;} else {Zinit = (float)lines[line].r * glDrawRatio;} //retract
                layersVertArraysPrt[comment][3][tmpUInt] = tmpFloat3; layersVertArraysPrt[comment][3][tmpUInt+1] = Zinit; layersVertArraysPrt[comment][3][tmpUInt+2] = tmpFloat5;
                layersVertArraysPrt[comment][3][tmpUInt+3] = tmpFloat3; layersVertArraysPrt[comment][3][tmpUInt+4] = tmpFloat4; layersVertArraysPrt[comment][3][tmpUInt+5] = tmpFloat5;
                if (gPrev == 81 || gPrev == 82 || gPrev == 83) { //move between drill
                    tmpUInt = layersVertIndex[comment][1]; layersVertIndex[comment][1] += 6;
                    layersVertArraysPrt[comment][1][tmpUInt] = tmpFloat0; layersVertArraysPrt[comment][1][tmpUInt+1] = Zinit; layersVertArraysPrt[comment][1][tmpUInt+2] = tmpFloat2;
                    layersVertArraysPrt[comment][1][tmpUInt+3] = tmpFloat3; layersVertArraysPrt[comment][1][tmpUInt+4] = Zinit; layersVertArraysPrt[comment][1][tmpUInt+5] = tmpFloat5;
                }
            }
        } line++;
    }

    for (unsigned int i=0; i<layersLimit; i++) {
        glBuildVertexArray (&layersVAO[i][0], &layersVBO[i][0], layersVertArraysPrt[i][0], layersVertCount[i][0]);
        glBuildVertexArray (&layersVAO[i][1], &layersVBO[i][1], layersVertArraysPrt[i][1], layersVertCount[i][1]);
        glBuildVertexArray (&layersVAO[i][2], &layersVBO[i][2], layersVertArraysPrt[i][2], layersVertCount[i][2]);
        glBuildVertexArray (&layersVAO[i][3], &layersVBO[i][3], layersVertArraysPrt[i][3], layersVertCount[i][3]);
    }

    //uniform buffer
    unsigned int UBOviewportMats; glGenBuffers(1, &UBOviewportMats); //generate uniform buffer
    glBindBuffer (GL_UNIFORM_BUFFER, UBOviewportMats); glBufferData (GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW); glBindBuffer (GL_UNIFORM_BUFFER, 0); //bind buffer
    glBindBufferRange (GL_UNIFORM_BUFFER, 0, UBOviewportMats, 0, 2 * sizeof(glm::mat4)); //assign buffer range

    //generate shader programs
    unsigned int shaderProgramGrid, shaderProgramSubGrid, shaderProgramAxisX, shaderProgramAxisY, shaderProgramAxisZ, shaderProgramLimits;
    unsigned int shaderProgramHighlight, shaderProgramG0, shaderProgramG1, shaderProgramG2, shaderProgramG81, tmpShaderProgram;
    if (generateShaderProgram (&shaderProgramGrid, vertexShaderLinesStr, fragmentShaderLinesStr, debugOutput) + \
    generateShaderProgram (&shaderProgramSubGrid, vertexShaderLinesStr, fragmentShaderLinesStr, debugOutput) + \
    generateShaderProgram (&shaderProgramAxisX, vertexShaderLinesStr, fragmentShaderLinesStr, debugOutput) + \
    generateShaderProgram (&shaderProgramAxisY, vertexShaderLinesStr, fragmentShaderLinesStr, debugOutput) + \
    generateShaderProgram (&shaderProgramAxisZ, vertexShaderLinesStr, fragmentShaderLinesStr, debugOutput) + \
    generateShaderProgram (&shaderProgramLimits, vertexShaderLinesStr, fragmentShaderLinesStr, debugOutput) + \
    generateShaderProgram (&shaderProgramHighlight, vertexShaderLinesStr, fragmentShaderLinesStr, debugOutput) + \
    generateShaderProgram (&shaderProgramG0, vertexShaderToolLinesStr, fragmentShaderToolLinesStr, debugOutput) + \
    generateShaderProgram (&shaderProgramG1, vertexShaderToolLinesStr, fragmentShaderToolLinesStr, debugOutput) + \
    generateShaderProgram (&shaderProgramG2, vertexShaderToolLinesStr, fragmentShaderToolLinesStr, debugOutput) + \
    generateShaderProgram (&shaderProgramG81, vertexShaderToolLinesStr, fragmentShaderToolLinesStr, debugOutput) < 0) {glfwTerminate(); return 1;}
    unsigned int* shaderProgramGptr [4] = {&shaderProgramG0, &shaderProgramG1, &shaderProgramG2, &shaderProgramG81}; //pointer to each tool shader programs

    //set shaders proper colors
    glUseProgram(shaderProgramGrid); glUniform4fv (glGetUniformLocation(shaderProgramGrid, "color"), 1, glColGrid);
    glUseProgram(shaderProgramSubGrid); glUniform4fv (glGetUniformLocation(shaderProgramSubGrid, "color"), 1, glColSubGrid);
    glUseProgram(shaderProgramAxisX); glUniform4fv (glGetUniformLocation(shaderProgramAxisX, "color"), 1, glColAxisX);
    glUseProgram(shaderProgramAxisY); glUniform4fv (glGetUniformLocation(shaderProgramAxisY, "color"), 1, glColAxisY);
    glUseProgram(shaderProgramAxisZ); glUniform4fv (glGetUniformLocation(shaderProgramAxisZ, "color"), 1, glColAxisZ);
    glUseProgram(shaderProgramLimits); glUniform4fv (glGetUniformLocation(shaderProgramLimits, "color"), 1, glColLimits);
    glUseProgram(shaderProgramHighlight); glUniform4fv (glGetUniformLocation(shaderProgramHighlight, "color"), 1, glColToolHighlight);
    for (unsigned int i=0; i<4; i++) {
        glUseProgram(*shaderProgramGptr[i]);
        glUniform4fv (glGetUniformLocation(*shaderProgramGptr[i], "color"), 1, glColTools[i]);
        glUniform1ui (glGetUniformLocation(*shaderProgramGptr[i], "clipPlaneYEn"), 1);
        glUniform1f (glGetUniformLocation(*shaderProgramGptr[i], "clipPlaneY"), (clippingYpos + 0.01) * glDrawRatio);
    }

    //default settings
    char strCfgBuffer [cfgVarsArrSize][12];
    char layerDisplayStr [100]; strcpy(layerDisplayStr, strGl[language][STR_GL::LAYER_DISPLAY]);
    char layerName [sizeof(ops[0].name) / sizeof(ops[0].name[0]) + 8];
    bool displayAxis = true, displayGrid = true, displaySubGrid = false /*disable subgrid by default*/, displayLimits = true, displayConfigWindow = false, displayAboutWindow = false, cfgBufferReset = true, sideBarLastElement = false; unsigned int bufferDrawn = 0;
    ImVec2 mainMenuBarSize = ImVec2 (0.f, 0.f), mainSideBarSize = ImVec2 (0.f, 0.f), mainSliderBarSize = ImVec2 (0.f, 0.f);
    double tmpDouble, tmpDouble1, opsDistArr[4], opsTimeArr[4]; //store temp data
    char toolpathName [5][30]; strcpy(toolpathName[0], strGl[language][STR_GL::TOOL_FAST]); strcpy(toolpathName[1], strGl[language][STR_GL::TOOL_WORK]); strcpy(toolpathName[2], strGl[language][STR_GL::TOOL_CIRC]); strcpy(toolpathName[3], strGl[language][STR_GL::TOOL_DRILL]); strcpy(toolpathName[4], strGl[language][STR_GL::TOOL_TOTAL]);
    char timeArr [] = "000h 00min";
    if (layersLimit==0 && strlen (ops[0].name) == 0) {strcpy(ops[0].name, strGl[language][STR_GL::NC_NOCOMMENT]);} //set default name if no comments
    
    //ImGui colors
    ImVec4 textColorArr[5] = {
        ImVec4 (glColTools[0][0], glColTools[0][1], glColTools[0][2], 1.f), //fast
        ImVec4 (glColTools[1][0], glColTools[1][1], glColTools[1][2], 1.f), //work
        ImVec4 (glColTools[2][0], glColTools[2][1], glColTools[2][2], 1.f), //circular
        ImVec4 (glColTools[3][0], glColTools[3][1], glColTools[3][2], 1.f), //drill
        ImVec4 (1.f, 1.f, 1.f, 1.f) //total
    };
    ImVec4 textInfoColor = ImVec4 (1.f, 1.f, 1.f, 1.f);
    ImVec4 textPercentColor = ImVec4 (1.f, 0.5f, 0.1f, 1.f);
    ImVec4 textSideBarColor = ImVec4 (.0f, .8f, .8f, 1.f);
    ImVec4 textClippingYColor = ImVec4 (0.2f, 1.f, 0.2f, 0.8f);
    ImVec4 textAxisXColor = ImVec4 (glColAxisX [0], glColAxisX [1], glColAxisX [2], 1.f);
    ImVec4 textAxisYColor = ImVec4 (glColAxisY [0], glColAxisY [1], glColAxisY [2], 1.f);
    ImVec4 textAxisZColor = ImVec4 (glColAxisZ [0], glColAxisZ [1], glColAxisZ [2], 1.f);

    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4 (.4f, .4f, .4f, 6.f)); //set lighter color for separator

    //ImGui window/buttons char array
    char cfgWindowTitle [strlen(strGl[language][STR_GL::MENU_CFG]) + 15]; sprintf (cfgWindowTitle ,"%s###cfgwindow", strGl[language][STR_GL::MENU_CFG]);
    char cfgBtnSave [strlen(strGl[language][STR_GL::WINDOW_SAVE]) + 13]; sprintf (cfgBtnSave ,"%s###cfgSave", strGl[language][STR_GL::WINDOW_SAVE]);
    char cfgBtnReset [strlen(strGl[language][STR_GL::WINDOW_RESETDEFAULT]) + 14]; sprintf (cfgBtnReset ,"%s###cfgReset", strGl[language][STR_GL::WINDOW_RESETDEFAULT]);
    char cfgBtnCancel [strlen(strGl[language][STR_GL::WINDOW_CANCEL]) + 15]; sprintf (cfgBtnCancel ,"%s###cfgCancel", strGl[language][STR_GL::WINDOW_CANCEL]);
    char aboutWindowTitle [strlen(strGl[language][STR_GL::MENU_ABOUT]) + 17]; sprintf (aboutWindowTitle ,"%s###aboutwindow", strGl[language][STR_GL::MENU_ABOUT]);
    char windowBtnClose [strlen(strGl[language][STR_GL::WINDOW_CLOSE]) + 16]; sprintf (windowBtnClose ,"%s###aboutClose", strGl[language][STR_GL::WINDOW_CLOSE]);

    //default conversion factors
    glCamAtDist = (tmpXmax - tmpXmin) * 0.05; //initial camera distance
    float keyboardSlide; //slide value when keyboard used

    //default camera position
    glCamYaw = deg2rad(30); glCamYawInit = glCamYaw; glCamPitch = deg2rad(45); glCamPitchInit = glCamPitch;
    glCamX = glCamAtX + sin(glCamYaw) * cos(glCamPitch) * glCamAtDist;
    glCamZ = glCamAtZ + cos(glCamYaw) * cos(glCamPitch) * glCamAtDist;
    glCamY = glCamAtY + sin(glCamPitch) * glCamAtDist;

    while (!glfwWindowShouldClose(window)) { //loop until the user closes the window
        if (cfgBufferReset) { //load current config settings
            for (unsigned int i = 0; i < cfgVarsArrSize; i++) {
                int tmpType = cfgVarsType[i];
                if (tmpType == 0) {sprintf (strCfgBuffer[i], "%d", *(int*)cfgVarsPtr[i]); //int
                } else if (tmpType == 1) {sprintf (strCfgBuffer[i], "%u", *(unsigned int*)cfgVarsPtr[i]); //unsigned int
                } else if (tmpType == 2) {sprintf (strCfgBuffer[i], "%f", *(float*)cfgVarsPtr[i]); //float
                } else if (tmpType == 3) {sprintf (strCfgBuffer[i], "%lf", *(double*)cfgVarsPtr[i]);} //double
            }
            cfgBufferReset = false; //resets for next loop
        }

        glfwPollEvents(); //poll for and process events

        //avoid glfw mouse callback conflict with ImGui
        if (!glImGuiFocused) {
            ImGuiMouseDelta = ImGuiCurrentIO.MouseDelta; ImGuiMouseWheel = ImGuiCurrentIO.MouseWheel;
            if (ImGui::IsMouseDown (ImGuiMouseButton_Left)) {glUpdateCamPosition (ImGuiMouseDelta.x, ImGuiMouseDelta.y, 0., 0., 0., false);} //left button pressed
            if (ImGui::IsMouseDown (ImGuiMouseButton_Right)) {glUpdateCamPosition (0., 0., -ImGuiMouseDelta.x, ImGuiMouseDelta.y, 0., false);} //right button pressed
            if (abs(ImGuiMouseWheel) > 0.001){glUpdateCamPosition (0., 0., 0., 0., ImGuiMouseWheel * glMouseScrollRatio * 15, false);} //scroll
        }

        //avoid glfw keyboard callback conflict with ImGui
        if (ImGui::IsKeyDown (GLFW_KEY_RIGHT_SHIFT) || ImGui::IsKeyDown (GLFW_KEY_LEFT_SHIFT)) {keyboardSlide = 6.f;} else {keyboardSlide = 3.f;} //double slide speed when shift key pressed
        if (ImGui::IsKeyDown (GLFW_KEY_UP) || ImGui::IsKeyDown (87)) {glUpdateCamPosition (0., 0., 0., 0., -keyboardSlide * glMouseScrollRatio, true);} //up or 'z': forward
        if (ImGui::IsKeyDown (GLFW_KEY_LEFT) || ImGui::IsKeyDown (64)) {glUpdateCamPosition (0, 0, -keyboardSlide, 0, 0, true);} //left or 'q': slide left
        if (ImGui::IsKeyDown (GLFW_KEY_RIGHT) || ImGui::IsKeyDown (68)) {glUpdateCamPosition (0, 0, keyboardSlide, 0, 0, true);} //right or 'd': slide right
        if (ImGui::IsKeyDown (GLFW_KEY_DOWN) || ImGui::IsKeyDown (83)) {glUpdateCamPosition (0., 0., 0., 0., keyboardSlide * glMouseScrollRatio, true);} //back or 's': backward
        if (ImGui::IsKeyDown (GLFW_KEY_ESCAPE)) {glfwSetWindowShouldClose (window, 1);} //'esc', quit program

        glfwGetWindowSize(window, &screenWidth, &screenHeight); //get viewport size
        //take menubar and sidebar in account
        if ((int)abs(screenHeight - mainSideBarSize.y) < 1) {screenWidth -= mainSideBarSize.x; screenXoff = (int)mainSideBarSize.x;} else {screenXoff = 0;}
        screenHeight -= mainMenuBarSize.y; screenWidth -= mainSliderBarSize.x; 

        if (screenWidth > 0 && screenHeight > 0 && (screenWidthLast != screenWidth || screenHeightLast != screenHeight)) { //screen size changed
            glViewportUpdate = true; //force update viewport
            glViewport(screenXoff, 0, screenWidth, screenHeight); //update viewport
            if (debugOutput) {printf ("screenWidth:%d, screenHeight:%d\n",screenWidth,screenHeight);}

            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.01f, 100.0f);
            glBindBuffer(GL_UNIFORM_BUFFER, UBOviewportMats); glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection)); glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glMouseScrollRatio = glCamSlideRatio = (glCamAtDist * atan(deg2rad(90) / 2) * ((float)screenWidth / (float)screenHeight)) / (float)screenWidth; //update scroll and slide ratio
        }

        if (glViewportForceUpdate || glViewportUpdate || bufferDrawn != 3) { //viewport needs update
            if (glViewportUpdate) {
                bufferDrawn = 0; glViewportUpdate = false; //toogle update vars
                glUpdateDepth (gridBondaries, &depthNear, &depthFar); //update depth values
            }
            bufferDrawn++;

            glCamView = glm::lookAt(glm::vec3(glCamX, glCamY, glCamZ), glm::vec3(glCamAtX, glCamAtY, glCamAtZ), glm::vec3(0.0f, 1.0f, 0.0f)); // camera/view transformation
            glBindBuffer(GL_UNIFORM_BUFFER, UBOviewportMats); glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glCamView)); glBindBuffer(GL_UNIFORM_BUFFER, 0); //bind uniform buffer
            
            glClearColor(0.3f,0.3f,0.3f,1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear current buffer
            if (displayLimits) {glUseProgram(shaderProgramLimits); glBindVertexArray(VAOlimits); glDrawArrays(GL_LINES, 0, verticesLimitsCount/3);} //draw limits
            if (displayAxis) { //draw axis
                glUseProgram(shaderProgramAxisZ); glBindVertexArray(VAOaxisZ); glDrawArrays(GL_LINE_STRIP, 0, verticesAxisCount/3); //z axis
                glUseProgram(shaderProgramAxisX); glBindVertexArray(VAOaxisX); glDrawArrays(GL_LINE_STRIP, 0, verticesAxisCount/3); //x axis
                glUseProgram(shaderProgramAxisY); glBindVertexArray(VAOaxisY); glDrawArrays(GL_LINE_STRIP, 0, verticesAxisCount/3); //y axis
            }

            // draw layers
            layersEnabled = 0; //reset layer enable count (ImGui menu)
            for (unsigned int i=0; i<layersLimit; i++) {
                if (layersEnable[i]) { //layer is enabled
                    layersEnabled++;
                    for (unsigned int j=0; j<4; j++) {
                        if (layersOpEnable[j]){ //operation enabled
                            if (layersHovered [i]) {tmpShaderProgram = shaderProgramHighlight;} else {tmpShaderProgram = *shaderProgramGptr[j];}
                            glUseProgram(tmpShaderProgram);
                            glUniform3f (glGetUniformLocation(tmpShaderProgram, "cam"), glCamX, glCamY, glCamZ);
                            glUniform1f (glGetUniformLocation(tmpShaderProgram, "near"), depthNear); glUniform1f (glGetUniformLocation(tmpShaderProgram, "far"), depthFar);
                            glBindVertexArray(layersVAO[i][j]); glDrawArrays(GL_LINES, 0, layersVertCount[i][j] / 3);
                        }
                    }
                }
            }

            if (displaySubGrid) {glUseProgram(shaderProgramSubGrid); glBindVertexArray(VAOsubGrid); glDrawArrays(GL_LINES, 0, verticesSubGridCount/3);} //draw subgrid
            if (displayGrid) {glUseProgram(shaderProgramGrid); glBindVertexArray(VAOgrid); glDrawArrays(GL_LINES, 0, verticesGridCount/3);} //draw grid
        }

        //ImGui frame
        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame(); //init ImGui frame

        if (ImGui::BeginMainMenuBar()) {
            mainMenuBarSize = ImGui::GetWindowSize(); //backup menu height for side tab positioning
            ImGui::SetCursorPosX (mainSideBarSize.x); //offset to avoid menu collide with side menu

            ImGui::Separator(); if (ImGui::BeginMenu(strGl[language][STR_GL::MENU_TOOLPATH])) { //toolpaths menu
                ImGui::PushStyleColor(ImGuiCol_Text, textColorArr[0]); ImGui::MenuItem(toolpathName[0], NULL, &layersOpEnable[0]); ImGui::PopStyleColor();
                ImGui::PushStyleColor(ImGuiCol_Text, textColorArr[1]); ImGui::MenuItem(toolpathName[1], NULL, &layersOpEnable[1]); ImGui::PopStyleColor();
                ImGui::PushStyleColor(ImGuiCol_Text, textColorArr[2]); ImGui::MenuItem(toolpathName[2], NULL, &layersOpEnable[2]); ImGui::PopStyleColor();
                ImGui::PushStyleColor(ImGuiCol_Text, textColorArr[3]); ImGui::MenuItem(toolpathName[3], NULL, &layersOpEnable[3]); ImGui::PopStyleColor();
                ImGui::EndMenu();
            }
            
            ImGui::Separator(); if (ImGui::BeginMenu(strGl[language][STR_GL::MENU_DISPLAY])) { //display menu
                if (ImGui::MenuItem(strGl[language][STR_GL::MENU_RESETVIEW], NULL, false)) {
                    glCamYaw = glCamYawInit; glCamPitch = glCamPitchInit; glCamAtX = glCamAtXinit; glCamAtY = glCamAtYinit; glCamAtZ = glCamAtZinit;
                    glUpdateCamPosition (0., 0., 0., 0., 0., true);
                }
                ImGui::MenuItem(strGl[language][STR_GL::MENU_AXIS], NULL, &displayAxis);
                ImGui::MenuItem(strGl[language][STR_GL::MENU_GRID], NULL, &displayGrid);
                ImGui::MenuItem(strGl[language][STR_GL::MENU_SUBGRID], NULL, &displaySubGrid);
                ImGui::MenuItem(strGl[language][STR_GL::MENU_LIMITS], NULL, &displayLimits);
                if(ImGui::MenuItem(strGl[language][STR_GL::LAYER_LAYERS], NULL, layersEnabled != 0)) {for (unsigned int i=0; i<layersLimit; i++){layersEnable[i]=(layersEnabled==0)?true:false;}}
                ImGui::EndMenu();
            }

            ImGui::Separator(); if (ImGui::BeginMenu(strGl[language][STR_GL::MENU_SETTINGS])) { //settings menu
                if (ImGui::BeginMenu(strGl[language][STR_GL::MENU_DISPLAY])) { //settings menu
                    if (ImGui::MenuItem(strGl[language][STR_GL::MENU_AA], NULL, &glAAenabled)) {(glAAenabled) ? glEnable(GL_LINE_SMOOTH) : glDisable(GL_LINE_SMOOTH); glViewportUpdate = true;}
                    ImGui::MenuItem(strGl[language][STR_GL::MENU_LIMITFPS], NULL, &glVsyncEnabled);
                    ImGui::MenuItem(strGl[language][STR_GL::MENU_FORCEUPD], NULL, &glViewportForceUpdate);
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem(strGl[language][STR_GL::MENU_CFG], NULL, false)) {displayConfigWindow = true;}
                if (ImGui::MenuItem(strGl[language][STR_GL::MENU_ABOUT], NULL, false)) {displayAboutWindow = true;}
                ImGui::EndMenu();
            }

            //draw Z clipping value
            ImGui::SetCursorPos (ImVec2 (0.f, -100.f)); ImGui::Text (clippingYstr); //draw offscreen to get proper item size
            ImVec2 clippingTextSize = ImGui::GetItemRectSize (); ImGui::SetCursorPos (ImVec2 (screenXoff + screenWidth - clippingTextSize.x + 5.f, 0.f));
            ImGui::TextColored (textClippingYColor, clippingYstr);

            ImGui::EndMainMenuBar();
        }

        //start sidebar
        ImGui::SetNextWindowPos(ImVec2 (0.f,0.f)); ImGui::SetNextWindowSize(ImVec2 (0.f,(float)screenHeight + mainMenuBarSize.y)); //set sidebar position and size
        ImGui::SetNextWindowCollapsed (true, ImGuiCond_Once); //collapse on first draw loop
        if (ImGui::Begin(strGl[language][STR_GL::LAYER_REPORT], NULL, sideBarFlags)) {
            mainSideBarSize = ImGui::GetWindowSize(); //backup sidebar sizes for menu positioning
            for (unsigned int i=0; i<layersLimit+1; i++) { //layers loop
                if (i < layersLimit) {tmpDouble = ops[i].distWork + ops[i].distFast + ops[i].distCircular + ops[i].distDrill; sideBarLastElement = false; //layer
                } else {tmpDouble = layerTotalDist[0] + layerTotalDist[1] + layerTotalDist[2] + layerTotalDist[3]; sideBarLastElement = true;} //total
                if (tmpDouble > 0.0001) { //something happened in current layer
                    if (sideBarLastElement) {ImGui::Separator(); sprintf (layerName, "%s  ###%d", strGl[language][STR_GL::TOOL_TOTAL], i); //total menu name
                    } else {sprintf (layerName, "%s  ###%d", ops[i].name,i);} //layers menu name
                    
                    if (ImGui::TreeNodeEx(layerName, (layersEnable[i]) ? ImGuiTreeNodeFlags_Selected : 0)) {
                        if (!sideBarLastElement && ImGui::IsItemHovered ()) {layersHovered [i] = true; glViewportUpdate = true;} else {layersHovered [i] = false;} //menu is howered, also avoid a glitch where menu disappair without notice
                        if (!sideBarLastElement){
                            sprintf (layerDisplayStr, "%s###dispLayer%d", (layersEnable[i]) ? strGl[language][STR_GL::LAYER_HIDE] : strGl[language][STR_GL::LAYER_DISPLAY], i); //layers menu name
                            ImGui::SameLine(); if (ImGui::SmallButton(layerDisplayStr)) {layersEnable[i] = !layersEnable[i];}
                        }
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 2.f)); //disable item spacing
                        ImGui::PushStyleColor(ImGuiCol_Text, textSideBarColor); //custom text color
                        if (!sideBarLastElement) { //layer
                            opsDistArr[0]=ops[i].distFast; opsDistArr[1]=ops[i].distWork; opsDistArr[2]=ops[i].distCircular; opsDistArr[3]=ops[i].distDrill;
                            opsTimeArr[0]=ops[i].timeFast; opsTimeArr[1]=ops[i].timeWork; opsTimeArr[2]=ops[i].timeCircular; opsTimeArr[3]=ops[i].timeDrill;
                        } else { //total
                            opsDistArr[0]=layerTotalDist[0]; opsDistArr[1]=layerTotalDist[1]; opsDistArr[2]=layerTotalDist[2]; opsDistArr[3]=layerTotalDist[3];
                            opsTimeArr[0]=layerTotalTime[0]; opsTimeArr[1]=layerTotalTime[1]; opsTimeArr[2]=layerTotalTime[2]; opsTimeArr[3]=layerTotalTime[3];
                        }

                        for (unsigned int j=0; j<5; j++) { //toolpaths
                            if (j == 4) {tmpDouble = opsDistArr[0] + opsDistArr[1]; tmpDouble1 = opsTimeArr[0] + opsTimeArr[1]; //total
                            } else {tmpDouble = opsDistArr[j]; tmpDouble1 = opsTimeArr[j];} //ops
                            if (tmpDouble > 0.0001) { //dist > 0
                                sec2charArr (timeArr, tmpDouble1 * 60);
                                ImGui::TextColored(textColorArr[j], toolpathName [j]);
                                ImGui::SameLine(); ImGui::Text(": "); ImGui::SameLine(); ImGui::TextColored(textInfoColor, "%.2lfmm", tmpDouble);
                                ImGui::SameLine(); ImGui::Text(", "); ImGui::SameLine(); ImGui::TextColored(textInfoColor, "%s", timeArr);
                                if (speedPercent!=100) {
                                    sec2charArr (timeArr, (tmpDouble1 / ((double)speedPercent / 100)) * 60);
                                    ImGui::SameLine(); ImGui::Text(", "); ImGui::SameLine(); ImGui::TextColored(textPercentColor, "%d%%", speedPercent);
                                    ImGui::SameLine(); ImGui::Text(": "); ImGui::SameLine(); ImGui::TextColored(textInfoColor, "%s", timeArr);
                                }
                            }
                        }
                        
                        ImGui::Separator(); //add separator at the end
                        ImGui::PopStyleVar(); ImGui::PopStyleColor(); //restore initial style
                        ImGui::TreePop();
                    } else {
                        if (!sideBarLastElement && ImGui::IsItemHovered ()) {layersHovered [i] = true; glViewportUpdate = true;} else {layersHovered [i] = false;} //menu is howered
                    }
                }
            }

            ImGui::Separator();
            if (ImGui::TreeNodeEx(strGl[language][STR_GL::MENU_LIMITS], (displayLimits) ? ImGuiTreeNodeFlags_Selected : 0)) { //limits
                sprintf (layerDisplayStr, "%s###dispLimits", (displayLimits) ? strGl[language][STR_GL::LAYER_HIDE] : strGl[language][STR_GL::LAYER_DISPLAY]);
                ImGui::SameLine(); if (ImGui::SmallButton(layerDisplayStr)) {displayLimits = !displayLimits;}
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 2.f)); //disable item spacing
                ImGui::TextColored(textAxisXColor, "X: "); ImGui::SameLine(); ImGui::Text("%.2lfmm <> %.2lfmm", limits->xMin, limits->xMax);
                ImGui::TextColored(textAxisYColor, "Y: "); ImGui::SameLine(); ImGui::Text("%.2lfmm <> %.2lfmm", limits->yMin, limits->yMax);
                ImGui::TextColored(textAxisZColor, "Z: "); ImGui::SameLine(); ImGui::Text("%.2lfmm <> %.2lfmm", limits->zMin, limits->zMax);
                ImGui::PopStyleVar();
                ImGui::TreePop();
            }
            
            ImGui::End();
        }

        //start sliderbar
        ImGui::SetNextWindowPos(ImVec2 (screenXoff + screenWidth, mainMenuBarSize.y)); ImGui::SetNextWindowSize(ImVec2 (12.f, (float)screenHeight)); //set position and size
        if (ImGui::Begin("###sliderwindow", NULL, sliderBarFlags)) {
            mainSliderBarSize = ImGui::GetWindowSize(); //backup sliderbar sizes for menu positioning
            ImGui::SetCursorPos (ImVec2 (0.f, 0.f));
            if (ImGui::VSliderFloat("###YclippingSlider", ImVec2 (10.f, screenHeight), &clippingYpos, (float)limits->zMin, (float)limits->zMax, "", ImGuiSliderFlags_NoInput)) {
                sprintf (clippingYstr, "%s: %.2lfmm", strGl[language][STR_GL::TOOL_ZCLIPPING], clippingYpos);
                for (unsigned int j=0; j<4; j++) {glUseProgram(*shaderProgramGptr[j]); glUniform1f (glGetUniformLocation(*shaderProgramGptr[j], "clipPlaneY"), (clippingYpos + 0.01) * glDrawRatio);} //operation loop
            }
            ImGui::End();
        }

        if (displayConfigWindow) { //display config window
            if (ImGui::Begin(cfgWindowTitle, NULL, configWindowFlags)) { //start ImGui frame
                char tmpVarsID [14];
                for (unsigned int i = 0; i < cfgVarsArrSize; i++) {
                    sprintf (tmpVarsID, "###cgrInp%d", i); //generate InputText id
                    ImGui::Text ("%s:", strGlCfg[language][i]);
                    ImGui::InputText (tmpVarsID, strCfgBuffer[i], 11, ImGuiInputTextFlags_CharsDecimal);
                }
                ImGui::Separator();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.9f, 0.f, 0.f, 1.0f)); ImGui::Text ("%s", strGl[language][STR_GL::CFG_APPLYWARN]); ImGui::PopStyleColor();
                ImGui::Separator();
                if (ImGui::Button (cfgBtnSave)) {
                    for (unsigned int i = 0; i < cfgVarsArrSize; i++) { //config buffer to var
                        int tmpType = cfgVarsType[i];
                        if (tmpType == 0) {*(int*)cfgVarsPtr[i] = atoi (strCfgBuffer[i]); //int
                        } else if (tmpType == 1) {*(unsigned int*)cfgVarsPtr[i] = atoi (strCfgBuffer[i]); //unsigned int
                        } else if (tmpType == 2) {*(float*)cfgVarsPtr[i] = atof (strCfgBuffer[i]); //float
                        } else if (tmpType == 3) {*(double*)cfgVarsPtr[i] = atof (strCfgBuffer[i]);} //double
                    }
                    configSave (false, false); 
                    displayConfigWindow = false;
                }

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 0.5f, 0.1f, 1.f));
                ImGui::SameLine (); if (ImGui::Button (cfgBtnReset)) {configSave (true); displayConfigWindow = false;}
                ImGui::PopStyleColor();

                ImGui::SameLine (); if (ImGui::Button (cfgBtnCancel)){displayConfigWindow = false;}
                ImGui::End();
            }
        }

        if (displayAboutWindow) { //display about window
            if (ImGui::Begin(aboutWindowTitle, NULL, configWindowFlags)) { //start ImGui frame
                ImGui::Text ("%s v%s: %s", "nc2png", programversion, "https://github.com/porcinus/nc2png");
                ImGui::Separator();
                ImGui::Text ("%s:", strGl[language][STR_GL::ABOUT_EXTLIB]);
                ImGui::Text ("\tlibGD (%s): https://libgd.github.io/",libGDver);
                ImGui::Text ("\tlibpng (%s): http://www.libpng.org/",libPNGver);
                ImGui::Text ("\tzlib (%s): https://zlib.net/",zlibver);
                ImGui::Separator();
                ImGui::Text ("%s:", "OpenGL");
                ImGui::Text ("\tGLFW (%d.%d.%d): https://www.glfw.org/", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
                ImGui::Text ("\tGLAD: https://glad.dav1d.de/");
                ImGui::Text ("\tGLM: https://github.com/g-truc/glm");
                ImGui::Text ("\tImGui (%s): https://github.com/ocornut/imgui", ImGui::GetVersion());
                ImGui::Text ("\tHuge thanks: https://learnopengl.com/Introduction");
                ImGui::Separator();
                if (ImGui::Button (windowBtnClose)){displayAboutWindow = false;}
                ImGui::End();
            }
        }

        glImGuiFocused = ImGui::IsAnyItemHovered() || ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow); //any ImGui instance hovered
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {ImGui::SetWindowFocus(NULL);} //reset focus
        ImGui::Render(); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); //render ImGui frame

        glfwSwapBuffers(window); //swap buffers

        gettimeofday(&tv, NULL); frameEndTV = 1000000 * tv.tv_sec + tv.tv_usec; //frame end time
        if (glVsyncEnabled) { //pseudo vsync, gdfw vsync drain a full core for nothing
            frameSleep = 16666.66666666667 - (frameEndTV - frameStartTV);
            if (frameSleep > 0) {usleep(frameSleep);} //sleep
            gettimeofday(&tv, NULL); frameStartTV = 1000000 * tv.tv_sec + tv.tv_usec; //frame start time
        }

        if (frameEndTV - glViewportLastUpdate > 100000){glViewportUpdate = true; glViewportLastUpdate = frameEndTV;} //limit viewport update to 10fps when nothing happen
        if (frameEndTV - glFPSlastTime > 1000000){ //1sec
            sprintf (windowTitle, "NC2PNG %s (%dfps)", programversion, glFPS); glfwSetWindowTitle (window, windowTitle); //update window title
            glFPS = 0; glFPSlastTime = frameEndTV; //reset fps counter and backup last time for fps counter
        } else {glFPS++;} //increment fps counter

        screenWidthLast = screenWidth, screenHeightLast = screenHeight; //backup screen size
    }

    if(debugOutput){printf ("GLFW window closing\n");}
    
    //cleanup
    delete verticesSubGrid; delete verticesGrid; delete verticesAxisX; delete verticesAxisY; delete verticesAxisZ, delete verticesLimits, delete gridBondaries;
    glDeleteBuffers(1, &VBOgrid); glDeleteBuffers(1, &VBOsubGrid); glDeleteBuffers(1, &VBOaxisX); glDeleteBuffers(1, &VBOaxisY); glDeleteBuffers(1, &VBOaxisZ); glDeleteBuffers(1, &VBOlimits);
    for (unsigned int i=0; i<layersLimit; i++) {
        delete layersVertArraysPrt[i][0]; delete layersVertArraysPrt[i][1]; delete layersVertArraysPrt[i][2]; delete layersVertArraysPrt[i][3]; //delete layers vertices arrays
        glDeleteBuffers(1, &layersVBO[i][0]); glDeleteBuffers(1, &layersVBO[i][1]); glDeleteBuffers(1, &layersVBO[i][2]); glDeleteBuffers(1, &layersVBO[i][3]); //delete vbo
    }
    glDeleteProgram(shaderProgramGrid); glDeleteProgram(shaderProgramSubGrid); glDeleteProgram(shaderProgramAxisX); glDeleteProgram(shaderProgramAxisY); glDeleteProgram(shaderProgramAxisZ);
    for (unsigned int j=0; j<4; j++) {glDeleteProgram(*shaderProgramGptr[j]);}

    //ImGui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate(); //terminate glfw
    return 0;
}

