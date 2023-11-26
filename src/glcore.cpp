/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to OpenGl window.

Important: If you update ImGui, please edit imgui_widgets.cpp and add in ImGui::BeginMainMenuBar() window_flags : " | ImGuiWindowFlags_NoBringToFrontOnFocus"
Note: This file is kind of a mess and may be reworked in the future.

upd 0.4a ok
*/

#include "glcore_language.h"
#include "glcore.h"

//#define depthBufferImageCount 256 //depthmap layers count

int strNumSepFormatingLength(LONG_TYPE num){ //detect number length including 1000 char separator and terminating null char
    int length = (num < 0) ? 2 : 1, pos = 0;
    num = abs(num);
    while (num > 0){
        num /= 10;
        length++;
        if (++pos == 3 && num){length++; pos = 0;}
    }
    return length;
}

void strNumSepFormating(char* str, int strLength, char separator, LONG_TYPE num){ //format number with given 1000 char separator and terminating null char
    if (str == nullptr){return;}
    int numLength = strNumSepFormatingLength(num);
    char *strPtr = str + ((numLength > strLength) ? strLength : numLength) - 1;
    *strPtr-- = '\0';
    bool negative = num < 0;
    num = abs(num);
    int pos = 0;
    while (num > 0 && strPtr >= str){
        *strPtr-- = (num % 10) + '0';
        num /= 10;
        if (++pos == 3 && num > 0 && strPtr >= str){
            *strPtr-- = separator;
            pos = 0;
        }
    }
    if (negative && strPtr >= str){*strPtr = '-';}
}


void glCallbackRefresh(GLFWwindow* window){ //force update viewport on context deterioration or window change
    glViewportUpdate = true;
}

int generateShaderProgramFromFile(unsigned int* program, char* vertexShaderFile, char* fragmentShaderFile, bool debugOutput){ //generate shader program from file
    int returnCode = 0;
    char buffer[4096]; //read line buffer

    //allocate buffers
    char* vertexBuffer = new(std::nothrow)char[shaderStrBufferSize + 1]();
    if (vertexBuffer == nullptr){
        debug_stderr("failed to allocate vertexBuffer\n");
        return -ENOMEM;
    }

    char* fragmentBuffer = new(std::nothrow)char[shaderStrBufferSize + 1]();
    if (fragmentBuffer == nullptr){
        debug_stderr("failed to allocate fragmentBuffer\n");
        delete []vertexBuffer;
        return -ENOMEM;
    }

    //read vertex shader
    FILE* filehandle = fopen(vertexShaderFile, "r");
    if (filehandle == NULL){
        debug_stderr("failed to read vertex shader '%s'\n", vertexShaderFile);
        returnCode = -ENOENT;
        goto funcGenShaderFileEnd;
    }
    while (fgets(buffer, 4095, filehandle)){strcat(vertexBuffer, buffer);}
    fclose(filehandle); filehandle = NULL;

    //read fragment shader
    filehandle = fopen(fragmentShaderFile, "r");
    if (filehandle == NULL){
        debug_stderr("failed to read fragment shader '%s'\n", fragmentShaderFile);
        returnCode = -ENOENT;
        goto funcGenShaderFileEnd;
    }
    while (fgets(buffer, 4095, filehandle)){strcat(fragmentBuffer, buffer);}
    fclose(filehandle);

    if (returnCode == 0){debug_stderr("Using external shaders\n");}
    returnCode = generateShaderProgram(program, vertexBuffer, fragmentBuffer, debugOutput);

    funcGenShaderFileEnd:;
    if(fragmentBuffer != nullptr){delete []fragmentBuffer;}
    if(vertexBuffer != nullptr){delete []vertexBuffer;}

    debug_stderr("Using shaders:\n- %s\n- %s\n%s\n", vertexShaderFile, fragmentShaderFile, (returnCode == 0) ? "successful" : "failed");

    return returnCode;
}

int generateShaderProgram(unsigned int* program, char* vertexShaderArr, char* fragmentShaderArr, bool debugOutput){ //generate shader program
    int glCompileSuccess = 0;
    char glLog[512]; 

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); //create shader
    glShaderSource(vertexShader, 1, &vertexShaderArr, NULL); glCompileShader(vertexShader); //attach and compile
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &glCompileSuccess); //get shader compilation status
    if (!glCompileSuccess){ //compile failed
        if(debugOutput){
            glGetShaderInfoLog(vertexShader, 512, NULL, glLog);
            printf("Vertex shader failed to compile : %s\n", glLog);
        }
        return -EINTR;
    }
    debug_stderr("Vertex shader compile success\n");

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //create shader
    glShaderSource(fragmentShader, 1, &fragmentShaderArr, NULL); glCompileShader(fragmentShader); //attach and compile
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &glCompileSuccess); //get shader compilation status
    if (!glCompileSuccess){ //compile failed
        if(debugOutput){
            glGetShaderInfoLog(fragmentShader, 512, NULL, glLog);
            printf("Fragment shader failed to compile : %s\n", glLog);
        }
        return -EINTR;
    }
    debug_stderr("Fragment shader compile success\n");

    *program = glCreateProgram(); glAttachShader(*program, vertexShader); glAttachShader(*program, fragmentShader); //create shader program and attach shader
    glLinkProgram(*program); //link shader program
    glGetProgramiv(*program, GL_LINK_STATUS, &glCompileSuccess); //get link status
    if (!glCompileSuccess){ //link failed
        if(debugOutput){
            glGetProgramInfoLog(*program, 512, NULL, glLog);
            printf("Failed to create shader program : %s\n", glLog);
        }
        return -EINTR;
    }

    //delete shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return 0; //success
}

void glBuildVertexArray(unsigned int* vao, unsigned int* vbo, float* verticesArr, int verticesArrSize, int pointsPerVertice){ //generate VAO and VBO arrays
    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, verticesArrSize * sizeof(float), verticesArr, GL_STATIC_DRAW);
    for (int i = 0, j = 0; i < pointsPerVertice / 2; i++, j += 3){
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, pointsPerVertice * sizeof(float), (void*)(j * sizeof(float)));
        glEnableVertexAttribArray(i);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glUpdateCamPosition(double rotXdiff, double rotYdiff, double slideXdiff, double slideYdiff, double zoomYdiff, bool forceUpdate){ //update camera position
    if (abs(slideXdiff) > 0.00001){ //update view slide x
        float tmpSlideAngleX = glCamYaw + 1.570795;
        float tmpSin = sin(tmpSlideAngleX) * slideXdiff * glCamSlideRatio, tmpCos = cos(tmpSlideAngleX) * slideXdiff * glCamSlideRatio;
        glCamAtX = glCamAtX + tmpSin; glCamAtZ = glCamAtZ + tmpCos;
        forceUpdate = true; //force update cam
    }
    
    if (abs(slideYdiff) > 0.00001){ //update view slide y
        float tmpSlideAngleY = glCamPitch + 1.570795;
        float tmpSinX = sin(glCamYaw), tmpCosX = cos(glCamYaw), tmpSinY = sin(tmpSlideAngleY), tmpCosY = cos(tmpSlideAngleY);
        glCamAtX = glCamAtX + tmpSinX * tmpCosY * slideYdiff * glCamSlideRatio;
        glCamAtZ = glCamAtZ + tmpCosX * tmpCosY * slideYdiff * glCamSlideRatio;
        glCamAtY = glCamAtY + tmpSinY * slideYdiff * glCamSlideRatio;
        forceUpdate = true; //force update cam
    }

    if (abs(rotXdiff) > 0.00001 || abs(rotYdiff) > 0.00001 || abs(zoomYdiff) > 0.00001 || forceUpdate){ //update view rotation or zoom
        float tmpGLCamX, tmpGLCamZ, tmpGLCamY;
        glCamYaw = glCamYaw - rotXdiff * glCamRotRatio;
        if (glCamYaw < -3.14159F){glCamYaw = 3.14158F;} if (glCamYaw > 3.14159F){glCamYaw = -3.14158F;} //yaw rollover
        tmpGLCamX = sin(glCamYaw); tmpGLCamZ = cos(glCamYaw);

        glCamPitch = glCamPitch + rotYdiff * glCamRotRatio;
        if (glCamPitch < -1.57079F){glCamPitch = -1.57078F;} if (glCamPitch > 1.57079F){glCamPitch = 1.57078F;} //limit pitch
        tmpGLCamY = sin(glCamPitch);
        float tmpcos = cos(glCamPitch);

        if (abs(zoomYdiff) > 0.00001){
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

void glUpdateDepth(float* bondaries, float* nearDist, float* farDist){ //update depth, for shader usage
    float tmpDist = 0.f, tmpNear = 999.f, tmpFar = -999.f;
    glm::vec3 tmpCam = {glCamX, glCamY, glCamZ};
    for (unsigned int i = 0; i < 8; i+=2){
        glm::vec3 tmpPos = {bondaries[i], 0.f, bondaries[i+1]};
        tmpDist = glm::distance (tmpPos, tmpCam);
        if (tmpDist > tmpFar){tmpFar = tmpDist;}
        if (tmpDist < tmpNear){tmpNear = tmpDist;}
    }
    if (tmpFar - tmpNear < 0.1f){tmpFar = tmpNear + 0.1f;}
    *nearDist = tmpNear;
    *farDist = tmpFar;
}


int glGenerateArcVertices(float* array, unsigned int startIndex, double centerX, double centerY, double zStart, double zEnd, double xSize, double ySize, double startAngle, double endAngle, double resolution){ //Generate vertices from arc data and add them to a given array, set array to NULL to avoid vertice generation and only return vertice count, only work normal to XZ
    if (numDiffDouble(startAngle, endAngle) < 0.00001 || xSize < 0.00001 || ySize < 0.00001){return 0;}
    if (resolution < 0.0001){resolution = 0.0001;}

    float angle1 = deg2rad(startAngle), angle2 = deg2rad(endAngle), angleDiff = angle2 - angle1;
    float arclenght = (abs(angle2 - angle1) * (xSize / 2) + abs(angle2 - angle1) * (ySize / 2)) / 2;
    int stepCount = ceil(arclenght / (float)resolution) + 1; if (stepCount == 0){stepCount++;}
    float stepAngle = abs(angleDiff) / (float)stepCount, hwidth = xSize / 2, hheight = ySize / 2;

    if (angle1 > angle2){
        float tmpFloat = angle1; angle1 = angle2; angle2 = tmpFloat;
        tmpFloat = zStart; zStart = zEnd; zEnd = tmpFloat;
    }

    float x, y, z, xLast, yLast, zLast, tmpx, tmpy;
    int createdVertice = 0, stepCurrent = 0; bool first = true;

    for (float currentAngle = angle1; currentAngle < angle2; currentAngle += stepAngle){
        if (array != NULL){ //used that way to count vertices without generating points
            //start
            if (first){
                tmpx = cos(currentAngle); tmpy = sin(currentAngle);
                x = centerX + tmpx * hwidth;
                y = centerY + tmpy * hheight;
                z = zStart;
                first = false;
            } else {x = xLast; y = yLast; z = zLast;}

            //end
            tmpx = cos(currentAngle + stepAngle); tmpy = sin(currentAngle + stepAngle);
            xLast = centerX + tmpx * hwidth;
            yLast = centerY + tmpy * hheight;
            zLast = NNSfloatFade(zStart, zEnd, (1.f / stepCount) * (++stepCurrent));

            array[startIndex + createdVertice + 0] = xLast;
            array[startIndex + createdVertice + 1] = zLast;
            array[startIndex + createdVertice + 2] = yLast;
            array[startIndex + createdVertice + 3] = x;
            array[startIndex + createdVertice + 4] = z;
            array[startIndex + createdVertice + 5] = y;
        }
        createdVertice += 6;
    }

    return createdVertice;
}

int glExportOBJ(char* file, float* verticesArray, int pointsPerVertice, int triangleCount, float offsetX, float offsetY, float offsetZ, float scaling){ //export vertices array to obj file
    if (file == nullptr || verticesArray == nullptr || triangleCount < 1){return ECANCELED;}

    double durationBench = get_time_double();
    FILE* fileHandle = fopen(file, "w");
    if (fileHandle == NULL){
        debug_stderr("failed to create file \"%s\"\n", file);
        return -ENOENT;
    }

    //try to use bigger buffer
    char *exportWriteBuffer = new(std::nothrow)char[streamBufferSize + 1];
    if (exportWriteBuffer != nullptr){
        debug_stderr("exportWriteBuffer allocated\n");
        setvbuf(fileHandle, exportWriteBuffer, _IOFBF, streamBufferSize);
    } else {debug_stderr("Warning: failed to allocate exportWriteBuffer\n");}

    //header
    fprintf(fileHandle, "# Created with NC2PNG %s\n", programversion);
    fprintf(fileHandle, "# %d triangles\n\n", triangleCount);

    float x, y, z;
    int verticeOffset = pointsPerVertice - 3;
    for (int triangle = 0, point = 1; triangle < triangleCount; triangle++, point += 3){
        for (int i = 0; i < 3; i++){
            x = (*(verticesArray++) + offsetX) * scaling;
            y = (*(verticesArray++) + offsetY) * scaling;
            z = (*(verticesArray++) + offsetZ) * scaling;
            verticesArray += verticeOffset;
            fprintf(fileHandle, "v %f %f %f\n", x, y, z);
        }
        fprintf(fileHandle, "f %d %d %d\n\n", point, point + 1, point + 2);
    }

    fclose(fileHandle);
    if (exportWriteBuffer != nullptr){delete []exportWriteBuffer;}
    durationBench = get_time_double() - durationBench;
    printfTerm(strGLCore[STR_GLCORE::GLCORE_EXPORT_TIME_REPORT][language], file, durationBench);

    return 0;
}

int glExportSTLbin(char* file, float* verticesArray, int pointsPerVertice, int triangleCount, float offsetX, float offsetY, float offsetZ, float scaling){ //export vertices array to binary stl file
    if (file == nullptr || verticesArray == nullptr || triangleCount < 1){return ECANCELED;}

    double durationBench = get_time_double();
    FILE* fileHandle = fopen(file, "wb");
    if (fileHandle == NULL){
        debug_stderr("failed to create file \"%s\"\n", file);
        return -ENOENT;
    }

    //try to use bigger buffer
    char *exportWriteBuffer = new(std::nothrow)char[streamBufferSize + 1];
    if (exportWriteBuffer != nullptr){
        debug_stderr("exportWriteBuffer allocated\n");
        setvbuf(fileHandle, exportWriteBuffer, _IOFBF, streamBufferSize);
    } else {debug_stderr("Warning: failed to allocate exportWriteBuffer\n");}

    //header
    char header[80] = {0};
    snprintf(header, 80, "Created with NC2PNG %s, %d triangles", programversion, triangleCount);
    fwrite(header, 1, 80, fileHandle);

    //triangle count
    fwrite(&triangleCount, 1, 4, fileHandle);

    int attribute = 0;
    float pts[4][3] = {0}; //normal, x, y, z
    const int ptsOrder[4] = {0, 2, 1, 3}; //normal, vertex:2 1 3 (2 before 1 to work with inverted normal)
    int verticeOffset = pointsPerVertice - 3;
    for (int triangle = 0; triangle < triangleCount; triangle++){
        for (int i = 1; i < 4; i++){
            pts[i][0] = (*(verticesArray++) + offsetX) * scaling; //x
            pts[i][1] = (*(verticesArray++) + offsetY) * scaling; //y
            pts[i][2] = (*(verticesArray++) + offsetZ) * scaling; //z
            verticesArray += verticeOffset;
        }

        //compute inverted normal
        float ux = pts[2][0] - pts[1][0], uy = pts[2][1] - pts[1][1], uz = pts[2][2] - pts[1][2];
        float vx = pts[3][0] - pts[1][0], vy = pts[3][1] - pts[1][1], vz = pts[3][2] - pts[1][2];
        pts[0][0] = uy * vz - uz * vy; //i
        pts[0][1] = uz * vx - ux * vz; //j
        pts[0][2] = ux * vy - uy * vx; //k
        float a = -(sqrt(pts[0][0] * pts[0][0] + pts[0][1] * pts[0][1] + pts[0][2] * pts[0][2])); //vector length
        pts[0][0] /= a; pts[0][1] /= a; pts[0][2] /= a; //normalize vector

        for (int i = 0; i < 4; i++){ //write normal, x, y, z
            fwrite(&pts[ptsOrder[i]][0], 1, 4, fileHandle);
            fwrite(&pts[ptsOrder[i]][1], 1, 4, fileHandle);
            fwrite(&pts[ptsOrder[i]][2], 1, 4, fileHandle);
        }
        fwrite(&attribute, 1, 2, fileHandle); //attribute
    }

    fclose(fileHandle);
    if (exportWriteBuffer != nullptr){delete []exportWriteBuffer;}
    durationBench = get_time_double() - durationBench;
    printfTerm(strGLCore[STR_GLCORE::GLCORE_EXPORT_TIME_REPORT][language], file, durationBench);

    return 0;
}

void glProgramClose(void){ //run when gl program closes
    if (glAlreadyKilled){return;}
    usleep(100000); //avoid potential garbage on tty output
    glfwTerminate();

    debug_stderr("memory cleanup\n");
    if (verticesGridPtr != nullptr){delete []verticesGridPtr; verticesGridPtr = nullptr;}
    if (verticesSubGridPtr != nullptr){delete []verticesSubGridPtr; verticesSubGridPtr = nullptr;}
    if (gridBondariesPtr != nullptr){delete []gridBondariesPtr; gridBondariesPtr = nullptr;}
    if (verticesAxisXPtr != nullptr){delete []verticesAxisXPtr; verticesAxisXPtr = nullptr;}
    if (verticesAxisYPtr != nullptr){delete []verticesAxisYPtr; verticesAxisYPtr = nullptr;}
    if (verticesAxisZPtr != nullptr){delete []verticesAxisZPtr; verticesAxisZPtr = nullptr;}
    if (verticesLimitsPtr != nullptr){delete []verticesLimitsPtr; verticesLimitsPtr = nullptr;}
    if (verticesHeightMapPtr != nullptr){delete []verticesHeightMapPtr; verticesHeightMapPtr = nullptr;}
    for (unsigned int i = 0; i < NCPARSER_COMMENT_ARRAY; i++){
        for (unsigned int j = 0; j < 4; j++){
            if (layersVertArraysPtr[i][j] != nullptr){delete []layersVertArraysPtr[i][j]; layersVertArraysPtr[i][j] = nullptr;}
        }
    }

    debug_stderr("glcode killed\n");
    glAlreadyKilled = true;
}

int glPreview(char* file, ncLineStruct* lines, ncToolStruct* tools, ncCommentStruct* comments, ncLimitStruct* limits, ncSummaryStruct* summary, ncArraySize* arrSizes, bool debugOutput){
    atexit(glProgramClose); //run on program exit

    debugBack = debug;
    debug = debugOutput;

    GLFWwindow* window; //glfw window handle
    int windowTitleSize = strlen(file) + strlen("NC2PNG v00.00.00x -  (000000fps)");
    char windowTitle[windowTitleSize];
    sprintf(windowTitle, "NC2PNG %s - %s", programversion, file); //window title

    int screenWidthLast = 0, screenHeightLast = 0, screenXoff = 0, screenWidth = 1, screenHeight = 1; //viewport size and offset

    struct timeval tv; gettimeofday(&tv, NULL); //timeval, used for pseudo Vsync and fps count
    double frameSleep; int glFPS = 0; unsigned long frameEndTV, frameStartTV, glFPSlastTime, glViewportLastUpdate;
    frameEndTV = frameStartTV = glFPSlastTime = glViewportLastUpdate = 1000000 * tv.tv_sec + tv.tv_usec; //init time

    if (!glfwInit()){
        debug_stderr("Failed to init GLFW\n");
        return -ENOMEM;
    }
    debug_stderr("GLFW init success\n");
    
    window = glfwCreateWindow(glViewportWidth, glViewportHeight, windowTitle, NULL, NULL); //create gl window
    if (!window){
        debug_stderr("Failed to create GLFW window\n");
        glProgramClose();
        return -ENOMEM;
    }
    debug_stderr("GLFW window created\n");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwMakeContextCurrent(window); //make the window's context current
    glfwSwapInterval(0); //disable glfw vsync

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        debug_stderr("Failed to initialize OpenGL context\n");
        glProgramClose();
        return -ENOMEM;
    }
    debug_stderr("OpenGL context init success\n");

    glEnable(GL_DEPTH_TEST); //enable z buffer
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (glAAenabled){glEnable(GL_LINE_SMOOTH);} //smooth lines
    glDisable(GL_CULL_FACE); //should be this by default, here just in case

    //ImGui init
    ImGui::CreateContext(); ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true); ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiWindowFlags sideBarFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
    ImGuiWindowFlags sliderBarFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration;
    ImGuiWindowFlags configWindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoNavFocus;
    ImGuiIO& ImGuiCurrentIO = ImGui::GetIO();
    ImGuiCurrentIO.IniFilename = NULL; //disable ini load/save
    ImGuiCurrentIO.WantCaptureKeyboard = true;
    ImGuiCurrentIO.WantCaptureMouse = true;
    ImVec2 ImGuiMouseDelta;
    float ImGuiMouseWheel;

    //generate shader programs
    unsigned int shaderProgramGrid, shaderProgramSubGrid, shaderProgramAxisX, shaderProgramAxisY, shaderProgramAxisZ, shaderProgramLimits;
    unsigned int shaderProgramHighlight, shaderProgramG0, shaderProgramG1, shaderProgramG2, shaderProgramG81, tmpShaderProgram;
    unsigned int shaderProgramHeightMap;

    {
        shaderType shaderPrograms[] = {
            {&shaderProgramGrid, 0},
            {&shaderProgramSubGrid, 0},
            {&shaderProgramAxisX, 0},
            {&shaderProgramAxisY, 0},
            {&shaderProgramAxisZ, 0},
            {&shaderProgramLimits, 0},
            {&shaderProgramHighlight, 0},
            {&shaderProgramG0, 1},
            {&shaderProgramG1, 1},
            {&shaderProgramG2, 1},
            {&shaderProgramG81, 1},
            {&shaderProgramHeightMap, 2},
        };
        int shaderProgramsSize = sizeof(shaderPrograms) / sizeof(shaderType);

        for (int i = 0; i < shaderProgramsSize; i++){
            int programType = shaderPrograms[i].shaderType;
            unsigned int* program = shaderPrograms[i].program;
            int tmpRet = generateShaderProgramFromFile(program, (char*)shaderPaths[programType][0][0], (char*)shaderPaths[programType][1][0], debugOutput);
            #ifndef DISABLE_GL_INTERNAL_SHADERS //used for debug
                if (tmpRet != 0){ //generate shader from file failed
                    debug_stderr("program %d, external shaders failed, falling to integrated shaders\n", i);
                    tmpRet = generateShaderProgram(program, (char*)shaderPaths[programType][0][1], (char*)shaderPaths[programType][1][1], debugOutput);
                }
            #endif

            if (tmpRet != 0){ //external and internal shaders failed
                debug_stderr("failed to compile required shaders for program %d\n", i);
                glProgramClose();
                return -ENOMEM;
            } else {
                debug_stderr("program %d compiled sucessfully\n", i);
            }
        }
    }
    unsigned int* shaderProgramGptr[4] = {&shaderProgramG0, &shaderProgramG1, &shaderProgramG2, &shaderProgramG81}; //pointer to each tool shader programs

    //declare glfw callbacks
    glfwSetWindowRefreshCallback(window, &glCallbackRefresh); //window refresh callback

    //build XY (gcode) XZ (gl) limits
    int tmpXmin = floor(limits->xMin - 11.), tmpXmax = ceil (limits->xMax + 11.);
    int tmpYmin = floor(limits->yMin - 11.), tmpYmax = ceil (limits->yMax + 11.);
    int tmpZmin = floor(limits->zMin - 11.), tmpZmax = ceil (limits->zMax + 11.);
    if (tmpXmin > -11){tmpXmin = -11;} if (tmpXmax < 11){tmpXmax = 11;}
    if (tmpYmin > -11){tmpYmin = -11;} if (tmpYmax < 11){tmpYmax = 11;}
    if (tmpZmin > -11){tmpZmin = -11;} if (tmpZmax < 11){tmpZmax = 11;}

    //update camera lookAt vars
    glCamAtX = ((tmpXmin + tmpXmax) / 2) * glDrawRatio; glCamAtXinit = glCamAtX;
    glCamAtZ = ((tmpYmin + tmpYmax) / 2) * -glDrawRatio; glCamAtZinit = glCamAtZ;
    glCamAtY = ((tmpZmin + tmpZmax) / 2) * glDrawRatio; glCamAtYinit = glCamAtY;

    //offset GL Y axis if heightmap is enabled to avoid aliasing
    //important note: done that way to avoid messy tricks with heightmap fragment shader
    bool glHeightMapEnable = summary->tools > 0;
    float linesYoffset = 0;
    if (glHeightMapEnable){linesYoffset = 0.005f;}

    //generate grid subgrid vertices array, VBA, VBO
    LONG_TYPE verticesCount = 0;
    unsigned int verticesGridIndex = 0, verticesSubGridIndex = 0, VBOgrid, VAOgrid, VBOsubGrid, VAOsubGrid;
    unsigned int verticesGridCount = (ceil((double)(tmpXmax - tmpXmin) / 10.) + 1) * 6 + (ceil((double)(tmpYmax - tmpYmin) / 10.) + 1) * 6; //add 2 vertices per axis to avoid crash linked to poor rounding
    unsigned int verticesSubGridCount = (tmpXmax - tmpXmin + 2) * 6 + (tmpYmax - tmpYmin + 2) * 6 - verticesGridCount; //add 2 vertices per axis to avoid crash linked to poor rounding
    verticesCount += verticesGridCount + verticesSubGridCount;

    //grid and subgrid vertices arrays, init to 0
    float *verticesGrid = new (std::nothrow) float [verticesGridCount]();
    float *verticesSubGrid = new (std::nothrow) float [verticesSubGridCount](); 
    verticesGridPtr = verticesGrid; verticesSubGridPtr = verticesSubGrid;
    if (verticesGridPtr == nullptr || verticesSubGridPtr == nullptr){
        debug_stderr("Failed to allocate verticesGrid or verticesSubGrid\n");
        glProgramClose();
        return -ENOMEM;
    }

    float *tmpPtrFloat; unsigned int *tmpPtrUInt; //pointers
    float tmpFloat0, tmpFloat1 = (float)-tmpYmax * glDrawRatio, tmpFloat2 = (float)-tmpYmin * glDrawRatio; //start end z
    for(int i = tmpXmin; i < tmpXmax; i++){
        tmpFloat0 = (float) i * glDrawRatio; //start end x
        if (i%10 == 0){ //grid
            tmpPtrUInt = &verticesGridIndex;
            tmpPtrFloat = verticesGrid + verticesGridIndex;
        } else { //subgrid
            tmpPtrUInt = &verticesSubGridIndex;
            tmpPtrFloat = verticesSubGrid + verticesSubGridIndex;
        }
        *tmpPtrFloat = *(tmpPtrFloat + 3) = tmpFloat0; //start end x
        *(tmpPtrFloat + 1) = *(tmpPtrFloat + 4) = linesYoffset;
        *(tmpPtrFloat + 2) = tmpFloat1; *(tmpPtrFloat + 5) = tmpFloat2; //start end z
        *tmpPtrUInt += 6; //increment vertice count
    }
    tmpFloat0 = (float)tmpXmin * glDrawRatio; tmpFloat2 = (float)tmpXmax * glDrawRatio; //start end  x
    for(int i = tmpYmin; i < tmpYmax; i++){
        tmpFloat1 = (float) -i * glDrawRatio; //start end z
        if (i%10 == 0){ //grid
            tmpPtrUInt = &verticesGridIndex;
            tmpPtrFloat = verticesGrid + verticesGridIndex;
        } else { //subgrid
            tmpPtrUInt = &verticesSubGridIndex;
            tmpPtrFloat = verticesSubGrid + verticesSubGridIndex;
        }
        *(tmpPtrFloat) = tmpFloat0; *(tmpPtrFloat + 3) = tmpFloat2; //start end x
        *(tmpPtrFloat + 1) = *(tmpPtrFloat + 4) = linesYoffset;
        *(tmpPtrFloat + 2) = *(tmpPtrFloat + 5) = tmpFloat1; //start end z
        *tmpPtrUInt += 6; //increment vertice count
    }
    glBuildVertexArray(&VAOgrid, &VBOgrid, verticesGrid, verticesGridCount, 3);
    glBuildVertexArray(&VAOsubGrid, &VBOsubGrid, verticesSubGrid, verticesSubGridCount, 3);

    //grid bondaries for depth shader
    float *gridBondaries = new (std::nothrow) float [8]{
        (float) limits->xMin * glDrawRatio, (float) limits->yMin * -glDrawRatio, //xmin, zmin
        (float) limits->xMin * glDrawRatio, (float) limits->yMax * -glDrawRatio, //xmin, zmax
        (float) limits->xMax * glDrawRatio, (float) limits->yMin * -glDrawRatio, //xmax, zmin
        (float) limits->xMax * glDrawRatio, (float) limits->yMax * -glDrawRatio //xmax, zmax
    };
    gridBondariesPtr = gridBondaries;
    if (gridBondariesPtr == nullptr){
        debug_stderr("Failed to allocate gridBondaries\n");
        glProgramClose();
        return -ENOMEM;
    }
    float depthNear = 0., depthFar = 0.;

    //Y axis toolpath clipping
    float clippingYpos = (float)limits->zMax;
    char clippingYstr[strlen(strGLCore[STR_GLCORE::GLCORE_TOOL_ZCLIPPING][language]) + 20];
    sprintf(clippingYstr, " %s: %.2lfmm", strGLCore[STR_GLCORE::GLCORE_TOOL_ZCLIPPING][language], clippingYpos);

    //generate axis array, VBA, VBO
    unsigned int verticesAxisCount = 24, VBOaxisX, VAOaxisX, VBOaxisY, VAOaxisY, VBOaxisZ, VAOaxisZ;
    float *verticesAxisX = new (std::nothrow) float [verticesAxisCount]{
        (float) tmpXmin * glDrawRatio, linesYoffset, 0.0f, (float) tmpXmax * glDrawRatio, linesYoffset, 0.0f, //start end x
        (float) (tmpXmax - 4.5) * glDrawRatio, linesYoffset, 2 * glDrawRatio, (float) (tmpXmax - 4.5) * glDrawRatio, linesYoffset, -2 * glDrawRatio, //arrow z+ z-
        (float) tmpXmax * glDrawRatio, linesYoffset, 0.0f, //end x
        (float) (tmpXmax - 4.5) * glDrawRatio, 2 * glDrawRatio + linesYoffset, 0.0f, (float) (tmpXmax - 4.5) * glDrawRatio, -2 * glDrawRatio + linesYoffset, 0.0f, //arrow y+ y-
        (float) tmpXmax * glDrawRatio, linesYoffset, 0.0f //end x
    };
    float *verticesAxisY = new (std::nothrow) float [verticesAxisCount]{
        0.0f, linesYoffset, (float) tmpYmin * -glDrawRatio, 0.0f, linesYoffset, (float) tmpYmax * -glDrawRatio, //start end y
        2 * glDrawRatio,  + linesYoffset, (float) (tmpYmax - 4.5) * -glDrawRatio, -2 * glDrawRatio, linesYoffset, (float) (tmpYmax - 4.5) * -glDrawRatio, //arrow x+ x-
        0.0f, linesYoffset, (float) tmpYmax * -glDrawRatio, //end y
        0.0f, 2 * glDrawRatio + linesYoffset, (float) (tmpYmax - 4.5) * -glDrawRatio, 0.0f, -2 * glDrawRatio + linesYoffset, (float) (tmpYmax - 4.5) * -glDrawRatio, //arrow z+ z-
        0.0f, linesYoffset, (float) tmpYmax * -glDrawRatio //end y
    };
    float *verticesAxisZ = new (std::nothrow) float [verticesAxisCount]{
        0.0f, (float) tmpZmin * glDrawRatio + linesYoffset, 0.0f, 0.0f, (float) tmpZmax * glDrawRatio + linesYoffset, 0.0f, //start end z
        2 * glDrawRatio, (float) (tmpZmax - 4.5) * glDrawRatio + linesYoffset, 0.0f, -2 * glDrawRatio, (float) (tmpZmax - 4.5) * glDrawRatio + linesYoffset, 0.0f, //arrow x+ x-
        0.0f, (float) tmpZmax * glDrawRatio + linesYoffset, 0.0f, //end z
        0.0f, (float) (tmpZmax - 4.5) * glDrawRatio + linesYoffset, 2 * glDrawRatio, 0.0f, (float) (tmpZmax - 4.5) * glDrawRatio + linesYoffset, -2 * glDrawRatio, //arrow y+ y-
        0.0f, (float) tmpZmax * glDrawRatio + linesYoffset, 0.0f //end z
    };
    verticesAxisXPtr = verticesAxisX; verticesAxisYPtr = verticesAxisY; verticesAxisZPtr = verticesAxisZ;
    if (verticesAxisXPtr == nullptr || verticesAxisYPtr == nullptr || verticesAxisZPtr == nullptr){
        debug_stderr("Failed to allocate verticesAxisX or verticesAxisY or verticesAxisZ\n");
        glProgramClose();
        return -ENOMEM;
    }
    verticesCount += verticesAxisCount * 3;
    glBuildVertexArray(&VAOaxisX, &VBOaxisX, verticesAxisX, verticesAxisCount, 3);
    glBuildVertexArray(&VAOaxisY, &VBOaxisY, verticesAxisY, verticesAxisCount, 3);
    glBuildVertexArray(&VAOaxisZ, &VBOaxisZ, verticesAxisZ, verticesAxisCount, 3);

    //generate limits array, VBA, VBO
    unsigned int verticesLimitsCount = 24, VBOlimits, VAOlimits;
    float *verticesLimits = new (std::nothrow) float [verticesLimitsCount]{
        (float) limits->xMin * glDrawRatio, linesYoffset, tmpYmin * -glDrawRatio, (float) limits->xMin * glDrawRatio, linesYoffset, tmpYmax * -glDrawRatio, //xmin
        (float) limits->xMax * glDrawRatio, linesYoffset, tmpYmin * -glDrawRatio, (float) limits->xMax * glDrawRatio, linesYoffset, tmpYmax * -glDrawRatio, //xmax
        tmpXmin * glDrawRatio, linesYoffset, (float) limits->yMin * -glDrawRatio, tmpXmax * glDrawRatio, linesYoffset, (float) limits->yMin * -glDrawRatio, //ymin
        tmpXmin * glDrawRatio, linesYoffset, (float) limits->yMax * -glDrawRatio, tmpXmax * glDrawRatio, linesYoffset, (float) limits->yMax * -glDrawRatio //ymax
    };
    verticesLimitsPtr = verticesLimits;
    if (verticesLimitsPtr == nullptr){
        debug_stderr("Failed to allocate verticesLimits\n");
        glProgramClose();
        return -ENOMEM;
    }
    verticesCount += verticesLimitsCount;
    glBuildVertexArray(&VAOlimits, &VBOlimits, verticesLimits, verticesLimitsCount, 3);

    //generate toolpath array, VBA, VBO
    unsigned int layersLimit = arrSizes->commentStrucLimit; //valid comment detected
    unsigned int layersVertCount [layersLimit][4] = {0}, layersVertIndex[layersLimit][4] = {0}, layersEnabled = 0; //vertices count/index per layer: g0, g1, g2-3, g81-83
    unsigned int layersVAO [layersLimit][4], layersVBO[layersLimit][4]; //layers vba, vbo: g0, g1, g2-3, g81-83
    bool layersEnable [layersLimit+1], layersHovered [layersLimit+1] = {false}, layersOpEnable [4] = {true,true,true,true};
    int g=-1, gPrev=-1, comment=0, tmpInt=0;
    unsigned int /*line=0, */tmpUInt=0;
    float Zinit = .0f, tmpFloat3 = .0f, tmpFloat4 = .0f, tmpFloat5 = .0f; //temp vars

    //detect proper layers vertices count
    for (unsigned int line = 1; line < arrSizes->lineStrucLimit; line++){
        g = lines[line].g;
        gPrev = lines[line-1].g;
        comment = lines[line].comment;
        if (g == 0){layersVertCount[
            comment][0] += 6;
            continue;
        }

        if (g == 1){layersVertCount[
            comment][1] += 6;
            continue;
        }

        if (g == 2 || g == 3){
            if (numDiffDouble (lines[line].startAngle, lines[line].endAngle) > 1.){
                tmpInt = glGenerateArcVertices(NULL, 0, lines[line].i * glDrawRatio, lines[line].j * -glDrawRatio, lines[line-1].z * glDrawRatio, lines[line].z * glDrawRatio, lines[line].radius * glDrawRatio * 2, lines[line].radius * glDrawRatio * 2, (g == 2) ? lines[line].startAngle : lines[line].endAngle, (g == 2) ? lines[line].endAngle : lines[line].startAngle, glDrawArcPrecision * glDrawRatio);
                if (tmpInt > 0){layersVertCount[comment][2] += tmpInt;}
            }
            if (numDiffDouble (lines[line].startAngle1, lines[line].endAngle1) > 1.){
                tmpInt = glGenerateArcVertices(NULL, 0, lines[line].i * glDrawRatio, lines[line].j * -glDrawRatio, lines[line-1].z * glDrawRatio, lines[line].z * glDrawRatio, lines[line].radius * glDrawRatio * 2, lines[line].radius * glDrawRatio * 2, (g == 2) ? lines[line].startAngle1 : lines[line].endAngle1, (g == 2) ? lines[line].endAngle1 : lines[line].startAngle1, glDrawArcPrecision * glDrawRatio);
                if (tmpInt > 0){layersVertCount[comment][2] += tmpInt;}
            }
            continue;
        }

        if (g == 81 || g == 82 || g == 83){
            layersVertCount[comment][3] += 6;
            if (gPrev == 81 || gPrev == 82 || gPrev == 83){layersVertCount[comment][1] += 6;}
        }
    }

    //create vertices arrays
    float* layersVertArrays[layersLimit][4]; //array pointers to all layers/kind
    debug_stderr("layersLimit:%d\n",layersLimit);
    for (unsigned int i=0; i<layersLimit; i++){ //nothrow used for debug
        debug_stderr("layersVertArrays:%d : [0]:%d, [1]:%d, [2]:%d, [3]:%d\n",i,layersVertCount[i][0],layersVertCount[i][1],layersVertCount[i][2],layersVertCount[i][3]);
        
        layersVertArrays[i][0] = new (std::nothrow) float [layersVertCount[i][0]]();
        layersVertArraysPtr[i][0] = layersVertArrays[i][0];
        if (layersVertArraysPtr[i][0] == nullptr){
            printf("Failed to allocate layersVertArrays[%d][0]\n", i);
            glProgramClose();
            return -ENOMEM;
        }

        layersVertArrays[i][1] = new (std::nothrow) float [layersVertCount[i][1]]();
        layersVertArraysPtr[i][1] = layersVertArrays[i][1];
        if (layersVertArraysPtr[i][1] == nullptr){
            printf("Failed to allocate layersVertArrays[%d][1]\n", i);
            glProgramClose();
            return -ENOMEM;
        }

        layersVertArrays[i][2] = new (std::nothrow) float [layersVertCount[i][2]]();
        layersVertArraysPtr[i][2] = layersVertArrays[i][2];
        if (layersVertArraysPtr[i][2] == nullptr){
            printf("Failed to allocate layersVertArrays[%d][2]\n", i);
            glProgramClose();
            return -ENOMEM;
        }

        layersVertArrays[i][3] = new (std::nothrow) float [layersVertCount[i][3]]();
        layersVertArraysPtr[i][3] = layersVertArrays[i][3];
        if (layersVertArraysPtr[i][3] == nullptr){
            printf("Failed to allocate layersVertArrays[%d][3]\n", i);
            glProgramClose();
            return -ENOMEM;
        }

        verticesCount += layersVertCount[i][0] + layersVertCount[i][1] + layersVertCount[i][2] + layersVertCount[i][3];
        layersEnable[i] = true;
        layersHovered[i] = false; //default layer bools
    }
    layersEnable[layersLimit] = true; //total line enable

    //generate vertices data (fast, work, circular, drill, heightmap)
    unsigned int VBOHeightMap, VAOHeightMap;
    LONG_TYPE verticesHeightMapCount = 0;
    float heightMapZoffset = 0/*-0.01*/; //offset heightmap vertice by given value to limit tool lines aliasing
    //float heightMapZerror = heightMapZoffset;
    {
        //depth layers specific
        NNSbwImagePtr bwDepthImage = nullptr;
        int depthIndexStart, depthIndexEnd;
        float toolDia = 0.f, toolAngle, zWorkDiff, layerSpacing;
        float heightMapScale, heightMapMargin, heightMapXoffset, heightMapYoffset;
        float heightMapXmin, heightMapXmax, heightMapYmin, heightMapYmax;
        float heightMapWidth, heightMapHeight;
        int heightMapWidthInt, heightMapHeightInt;
        if (glHeightMapEnable){
            debug_stderr("glHeightMapEnable is enabled\n");
            zWorkDiff = limits->zMaxWork - limits->zMinWork;
            heightMapScale = 1.f / glHeightMapPrecision;
            layerSpacing = abs(zWorkDiff / 256) * heightMapScale;
            heightMapMargin = (limits->blockDetected) ? 0.f : limits->toolMax / 2.f + 5.f;
            heightMapXmin = limits->xMin; heightMapXmax = limits->xMax;
            heightMapYmin = limits->yMin; heightMapYmax = limits->yMax;
            heightMapXoffset = abs(heightMapXmin) + heightMapMargin;
            heightMapYoffset = abs(heightMapYmin) + heightMapMargin;
            heightMapWidth = (abs(heightMapXmax - heightMapXmin) + heightMapMargin * 2.f) * heightMapScale;
            heightMapHeight = (abs(heightMapYmax - heightMapYmin) + heightMapMargin * 2.f) * heightMapScale;
            heightMapWidthInt = ceil(heightMapWidth);
            heightMapHeightInt = ceil(heightMapHeight);
            debug_stderr("heightMapWidth:%.03f, heightMapHeight:%.03f\n", heightMapWidth, heightMapHeight);

            bwDepthImage = NNSbwImageCreate(heightMapWidthInt, heightMapHeightInt);
            glHeightMapEnable = bwDepthImage != nullptr;
        }

        if (glHeightMapEnable && ((int)heightMapWidthInt < 1 || (int)heightMapHeightInt < 1)){ //invalid size
            NNSbwImageDestroy(bwDepthImage);
            glHeightMapEnable = false;
            debug_stderr("invald heightMapWidth:%d or heightMapHeight:%d\n", heightMapWidthInt, heightMapHeightInt);
        }
        
        //progress bar
        printfTerm(strGLCore[STR_GLCORE::GLCORE_PROGRESS][language], 0, arrSizes->lineStrucLimit);
        int progressPosition = 0;

        for (unsigned int line = 1; line < arrSizes->lineStrucLimit; line++, progressPosition++){
            g = lines[line].g;
            gPrev = lines[line-1].g;
            comment = lines[line].comment;

            tmpFloat0 = (float)lines[line-1].x * glDrawRatio;
            tmpFloat1 = (float)lines[line-1].z * glDrawRatio;
            tmpFloat2 = (float)lines[line-1].y * glDrawRatio * -1;

            tmpFloat3 = (float)lines[line].x * glDrawRatio;
            tmpFloat4 = (float)lines[line].z * glDrawRatio;
            tmpFloat5 = (float)lines[line].y * glDrawRatio * -1;

            if (glHeightMapEnable){
                depthIndexStart = 255 - (255 * ((lines[line - 1].z - limits->zMinWork) / zWorkDiff));
                depthIndexEnd = 255 - (255 * ((lines[line].z - limits->zMinWork) / zWorkDiff));
                toolDia = tools[lines[line].tool].diameter * heightMapScale;
                toolAngle = tools[lines[line].tool].angle;
            }

            if ((g == 0 || g == 1)){
                tmpUInt = layersVertIndex[comment][g]; layersVertIndex[comment][g] += 6;
                layersVertArrays[comment][g][tmpUInt + 0] = tmpFloat0; //x-1
                layersVertArrays[comment][g][tmpUInt + 1] = tmpFloat1 + linesYoffset; //z-1
                layersVertArrays[comment][g][tmpUInt + 2] = tmpFloat2; //y-1

                layersVertArrays[comment][g][tmpUInt + 3] = tmpFloat3; //x
                layersVertArrays[comment][g][tmpUInt + 4] = tmpFloat4 + linesYoffset; //z
                layersVertArrays[comment][g][tmpUInt + 5] = tmpFloat5; //y
                if (glHeightMapEnable && (g == 0 || g == 1)){ //height map
                    float x1 = (lines[line - 1].x + heightMapXoffset) * heightMapScale, y1 = heightMapHeight - (lines[line - 1].y + heightMapYoffset) * heightMapScale;
                    float x2 = (lines[line].x + heightMapXoffset) * heightMapScale, y2 = heightMapHeight - (lines[line].y + heightMapYoffset) * heightMapScale;
                    if (toolAngle < 0.01f){ //no tool angle
                        NNSbwLineThickDepth(bwDepthImage, heightMapWidthInt, heightMapHeightInt, depthIndexStart, depthIndexEnd, x1, y1, x2, y2, toolDia);
                        goto toolVerticesEnd;
                    }

                    //tool angle
                    NNSbwLineThickVDepth(bwDepthImage, heightMapWidthInt, heightMapHeightInt, depthIndexStart, depthIndexEnd, x1, y1, x2, y2, toolDia, toolAngle, layerSpacing);
                }
                goto toolVerticesEnd;
            }
            
            if (g == 2 || g == 3){
                tmpUInt = layersVertIndex[comment][2];
                float i, j, radius;
                if (glHeightMapEnable){
                    i = (lines[line].i + heightMapXoffset) * heightMapScale;
                    j = heightMapHeight - (lines[line].j + heightMapYoffset) * heightMapScale;
                    radius = lines[line].radius * heightMapScale * 2;
                }

                if (numDiffDouble(lines[line].startAngle, lines[line].endAngle) > 1.){
                    tmpInt = glGenerateArcVertices(layersVertArrays[comment][2], layersVertIndex[comment][2], lines[line].i * glDrawRatio, lines[line].j * -glDrawRatio, lines[line-1].z * glDrawRatio + linesYoffset, lines[line].z * glDrawRatio + linesYoffset, lines[line].radius * glDrawRatio * 2, lines[line].radius * glDrawRatio * 2, (g == 2) ? lines[line].startAngle : lines[line].endAngle, (g == 2) ? lines[line].endAngle : lines[line].startAngle, glDrawArcPrecision * glDrawRatio);
                    if (tmpInt > 0){layersVertIndex[comment][2] += tmpInt;}
                    if (glHeightMapEnable){ //height map
                        if (toolAngle < 0.01f){ //no tool angle
                            NNSbwArcThickDepth(bwDepthImage, heightMapWidthInt, heightMapHeightInt, depthIndexStart, depthIndexEnd, i, j, radius, radius, (g == 2) ? lines[line].startAngle : lines[line].endAngle, (g == 2) ? lines[line].endAngle : lines[line].startAngle, toolDia, glDrawArcPrecision);
                        } else { //tool angle
                            NNSbwArcThickVDepth(bwDepthImage, heightMapWidthInt, heightMapHeightInt, depthIndexStart, depthIndexEnd, i, j, radius, radius, (g == 2) ? lines[line].startAngle : lines[line].endAngle, (g == 2) ? lines[line].endAngle : lines[line].startAngle, toolDia, toolAngle, layerSpacing, glDrawArcPrecision);
                        }
                    }
                }

                if (numDiffDouble(lines[line].startAngle1, lines[line].endAngle1) > 1.){
                    tmpInt = glGenerateArcVertices(layersVertArrays[comment][2], layersVertIndex[comment][2], lines[line].i * glDrawRatio, lines[line].j * -glDrawRatio, lines[line-1].z * glDrawRatio + linesYoffset, lines[line].z * glDrawRatio + linesYoffset, lines[line].radius * glDrawRatio * 2, lines[line].radius * glDrawRatio * 2, (g == 2) ? lines[line].startAngle1 : lines[line].endAngle1, (g == 2) ? lines[line].endAngle1 : lines[line].startAngle1, glDrawArcPrecision * glDrawRatio);
                    if (tmpInt > 0){layersVertIndex[comment][2] += tmpInt;}
                    if (glHeightMapEnable){ //height map
                        if (toolAngle < 0.01f){ //no tool angle
                            NNSbwArcThickDepth(bwDepthImage, heightMapWidthInt, heightMapHeightInt, depthIndexStart, depthIndexEnd, i, j, radius, radius, (g == 2) ? lines[line].startAngle1 : lines[line].endAngle1, (g == 2) ? lines[line].endAngle1 : lines[line].startAngle1, toolDia, glDrawArcPrecision);
                        } else { //tool angle
                            NNSbwArcThickVDepth(bwDepthImage, heightMapWidthInt, heightMapHeightInt, depthIndexStart, depthIndexEnd, i, j, radius, radius, (g == 2) ? lines[line].startAngle1 : lines[line].endAngle1, (g == 2) ? lines[line].endAngle1 : lines[line].startAngle1, toolDia, toolAngle, layerSpacing, glDrawArcPrecision);
                        }
                    }
                }
                goto toolVerticesEnd;
            }
            
            if (g == 81 || g == 82 || g == 83){
                tmpUInt = layersVertIndex[comment][3]; layersVertIndex[comment][3] += 6;
                Zinit = (lines[line].retractMode == 1) ? lines[line].startAngle * glDrawRatio : lines[line].r * glDrawRatio;

                layersVertArrays[comment][3][tmpUInt + 0] = tmpFloat3; //x
                layersVertArrays[comment][3][tmpUInt + 1] = Zinit + linesYoffset;
                layersVertArrays[comment][3][tmpUInt + 2] = tmpFloat5; //y

                layersVertArrays[comment][3][tmpUInt + 3] = tmpFloat3; //x
                layersVertArrays[comment][3][tmpUInt + 4] = tmpFloat4 + linesYoffset; //z
                layersVertArrays[comment][3][tmpUInt + 5] = tmpFloat5; //y

                if (gPrev == 81 || gPrev == 82 || gPrev == 83){ //move between drill
                    tmpUInt = layersVertIndex[comment][1]; layersVertIndex[comment][1] += 6;
                    layersVertArrays[comment][1][tmpUInt + 0] = tmpFloat0; //x-1
                    layersVertArrays[comment][1][tmpUInt + 1] = Zinit + linesYoffset;
                    layersVertArrays[comment][1][tmpUInt + 2] = tmpFloat2; //y-1

                    layersVertArrays[comment][1][tmpUInt + 3] = tmpFloat3; //x
                    layersVertArrays[comment][1][tmpUInt + 4] = Zinit + linesYoffset;
                    layersVertArrays[comment][1][tmpUInt + 5] = tmpFloat5; //y
                }

                if (glHeightMapEnable){ //height map
                    float x2 = (lines[line].x + heightMapXoffset) * heightMapScale;
                    float y2 = heightMapHeight - (lines[line].y + heightMapYoffset) * heightMapScale;
                    if (toolAngle < 0.01f){ //no tool angle
                        NNSbwCircleFilled(bwDepthImage, x2, y2, toolDia, depthIndexEnd);
                        goto toolVerticesEnd;
                    }
                    
                    //tool angle
                    NNSbwLineThickVDepth(bwDepthImage, heightMapWidthInt, heightMapHeightInt, depthIndexStart, depthIndexEnd, x2, y2, x2, y2, toolDia, toolAngle, layerSpacing);
                }
            }

            toolVerticesEnd:;
            if (progressPosition > 999){
                printf("\033[0G");
                printfTerm(strGLCore[STR_GLCORE::GLCORE_PROGRESS][language], line, arrSizes->lineStrucLimit);
                progressPosition = 0;
            }
        }

        printf("\033[0G\033[2K"); //remove progress bar

        //build height map
        if (glHeightMapEnable){
            double benchmarkStart = get_time_double();
            debug_stderr("starting process heightmap data\n");
            verticesHeightMapCount = (heightMapWidthInt + 1) * (heightMapHeightInt + 1) * 6 * 3 * 2; //extra room to avoid rounding issues

            debug_stderr("verticesHeightMapCount:" LONG_TYPE_FORMAT "\n", verticesHeightMapCount);

            float *verticesHeightMap = new (std::nothrow) float [verticesHeightMapCount](); 
            verticesHeightMapPtr = verticesHeightMap;

            if (verticesHeightMapPtr != nullptr){ //generate height map vertices
                heightMapXoffset *= heightMapScale;
                heightMapYoffset = (heightMapYoffset - heightMapHeight * glHeightMapPrecision) * heightMapScale;
                float heightMapXsubOffset = -glHeightMapPrecision * heightMapScale, heightMapYsubOffset = glHeightMapPrecision * heightMapScale; //todo fix proper way to offset
                float *verticesHeightMapTmpPtr = verticesHeightMapPtr;
                bool triangleSide = false; //used to alternate triangle direction
                for (int y = 1; y < heightMapHeightInt; y++){
                    for (int x = 1; x < heightMapWidthInt; x++, triangleSide = !triangleSide){
                        //xy   z
                        //0|   0|1
                        //---  ---
                        // |1  2|3
                        float x0 = (((float)(x - 1) - (heightMapXoffset + heightMapXsubOffset)) * glHeightMapPrecision) * glDrawRatio;
                        float y0 = (((float)(y - 1) + (heightMapYoffset + heightMapYsubOffset)) * glHeightMapPrecision) * glDrawRatio;
                        float x1 = (((float)x - (heightMapXoffset + heightMapXsubOffset)) * glHeightMapPrecision) * glDrawRatio;
                        float y1 = (((float)y + (heightMapYoffset + heightMapYsubOffset)) * glHeightMapPrecision) * glDrawRatio;

                        float z0 = NNSfloatFadeNoClamp(limits->zMinWork, limits->zMaxWork, (1.f - (float)NNSbwGetPixel(bwDepthImage, x - 1, y - 1) / 255)) * glDrawRatio + heightMapZoffset;
                        float z1 = NNSfloatFadeNoClamp(limits->zMinWork, limits->zMaxWork, (1.f - (float)NNSbwGetPixel(bwDepthImage, x, y - 1) / 255)) * glDrawRatio + heightMapZoffset;
                        float z2 = NNSfloatFadeNoClamp(limits->zMinWork, limits->zMaxWork, (1.f - (float)NNSbwGetPixel(bwDepthImage, x - 1, y) / 255)) * glDrawRatio + heightMapZoffset;
                        float z3 = NNSfloatFadeNoClamp(limits->zMinWork, limits->zMaxWork, (1.f - (float)NNSbwGetPixel(bwDepthImage, x, y) / 255)) * glDrawRatio + heightMapZoffset;
                        
                        float pts[6][3];
                        if (triangleSide){
                            pts[0][0] = x1; pts[0][1] = z1; pts[0][2] = y0; //1
                            pts[1][0] = x1; pts[1][1] = z3; pts[1][2] = y1; //3
                            pts[2][0] = x0; pts[2][1] = z0; pts[2][2] = y0; //0
                            pts[3][0] = x0; pts[3][1] = z2; pts[3][2] = y1; //2
                            pts[4][0] = x0; pts[4][1] = z0; pts[4][2] = y0; //0
                            pts[5][0] = x1; pts[5][1] = z3; pts[5][2] = y1; //3
                        } else {
                            pts[0][0] = x0; pts[0][1] = z0; pts[0][2] = y0; //0
                            pts[1][0] = x1; pts[1][1] = z1; pts[1][2] = y0; //1
                            pts[2][0] = x0; pts[2][1] = z2; pts[2][2] = y1; //2
                            pts[3][0] = x1; pts[3][1] = z3; pts[3][2] = y1; //3
                            pts[4][0] = x0; pts[4][1] = z2; pts[4][2] = y1; //2
                            pts[5][0] = x1; pts[5][1] = z1; pts[5][2] = y0; //1
                        }

                        //compute normal
                        float ptsNormal[2][3];
                        for (int i = 0, ptsOff = 0; i < 2; i++, ptsOff += 3){
                            float ux = pts[1 + ptsOff][0] - pts[0 + ptsOff][0], uy = pts[1 + ptsOff][1] - pts[0 + ptsOff][1], uz = pts[1 + ptsOff][2] - pts[0 + ptsOff][2];
                            float vx = pts[2 + ptsOff][0] - pts[0 + ptsOff][0], vy = pts[2 + ptsOff][1] - pts[0 + ptsOff][1], vz = pts[2 + ptsOff][2] - pts[0 + ptsOff][2];
                            float tmpPtsNormal[3] = {uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx}; //i, j, k
                            float a = -(sqrt(tmpPtsNormal[0] * tmpPtsNormal[0] + tmpPtsNormal[1] * tmpPtsNormal[1] + tmpPtsNormal[2] * tmpPtsNormal[2])); //vector length
                            ptsNormal[i][0] = tmpPtsNormal[0] / a;
                            ptsNormal[i][1] = tmpPtsNormal[1] / a;
                            ptsNormal[i][2] = tmpPtsNormal[2] / a;
                        }

                        //triangle loop
                        for (int i = 0; i < 2; i++){
                            for (int j = 0, jOff = i * 3; j < 3; j++){
                                int ptsNum = j + jOff;
                                *(verticesHeightMapTmpPtr++) = pts[ptsNum][0];
                                *(verticesHeightMapTmpPtr++) = pts[ptsNum][1];
                                *(verticesHeightMapTmpPtr++) = pts[ptsNum][2];
                                *(verticesHeightMapTmpPtr++) = ptsNormal[i][0];
                                *(verticesHeightMapTmpPtr++) = ptsNormal[i][1];
                                *(verticesHeightMapTmpPtr++) = ptsNormal[i][2];
                            }
                        }
                    }
                    //debug_stderr("pos:%d, y:%d\n", verticesHeightMapTmpPtr - verticesHeightMapPtr, y);
                }
                verticesHeightMapCount = verticesHeightMapTmpPtr - verticesHeightMapPtr + 1;

                if (verticesHeightMapCount){
                    verticesCount += verticesHeightMapCount;
                    glBuildVertexArray(&VAOHeightMap, &VBOHeightMap, verticesHeightMap, verticesHeightMapCount, 6);
                } else {
                    glHeightMapEnable = false;
                }

                if (debugOutput){
                    gdImagePtr gdPrevDepthCombinedImage = gdImageCreateTrueColor(heightMapWidthInt, heightMapHeightInt);
                    if (gdPrevDepthCombinedImage != NULL){
                        gdImageSaveAlpha(gdPrevDepthCombinedImage, true);
                        NNSgdImageFill(gdPrevDepthCombinedImage, gdTrueColorAlpha(255, 255, 255, gdAlphaOpaque));
                        NNSbwImage2gdGreyscaleCopy(bwDepthImage, gdPrevDepthCombinedImage);

                        char previewFilePath[PATH_MAX];
                        sprintf(previewFilePath, "%s.depthColorGL.png", file);
                        FILE* gdFileHandle = fopen(previewFilePath, "wb");
                        if (gdFileHandle != NULL){
                            gdImagePng(gdPrevDepthCombinedImage, gdFileHandle);
                            fclose(gdFileHandle);
                        }
                        gdImageDestroy(gdPrevDepthCombinedImage);
                    }
                }
                NNSbwImageDestroy(bwDepthImage);

                debug_stderr("heightmap duration : %.04lfs\n", get_time_double() - benchmarkStart);
            } else {
                debug_stderr("Failed to allocate verticesHeightMap\n");
                glHeightMapEnable = false;
            }
        }
    }

    for (unsigned int i=0; i < layersLimit; i++){
        glBuildVertexArray(&layersVAO[i][0], &layersVBO[i][0], layersVertArrays[i][0], layersVertCount[i][0], 3);
        glBuildVertexArray(&layersVAO[i][1], &layersVBO[i][1], layersVertArrays[i][1], layersVertCount[i][1], 3);
        glBuildVertexArray(&layersVAO[i][2], &layersVBO[i][2], layersVertArrays[i][2], layersVertCount[i][2], 3);
        glBuildVertexArray(&layersVAO[i][3], &layersVBO[i][3], layersVertArrays[i][3], layersVertCount[i][3], 3);
    }

    debug_stderr("verticesCount:" LONG_TYPE_FORMAT "\n", verticesCount);

    //uniform buffer
    unsigned int UBOviewportMats;
    glGenBuffers(1, &UBOviewportMats); //generate uniform buffer
    glBindBuffer(GL_UNIFORM_BUFFER, UBOviewportMats); //bind buffer
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW); //projection, view
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBOviewportMats, 0, 3 * sizeof(glm::mat4)); //assign buffer range

    //set shaders proper colors
    glUseProgram(shaderProgramGrid); glUniform4fv(glGetUniformLocation(shaderProgramGrid, "color"), 1, glColGrid);
    glUseProgram(shaderProgramSubGrid); glUniform4fv(glGetUniformLocation(shaderProgramSubGrid, "color"), 1, glColSubGrid);
    glUseProgram(shaderProgramAxisX); glUniform4fv(glGetUniformLocation(shaderProgramAxisX, "color"), 1, glColAxisX);
    glUseProgram(shaderProgramAxisY); glUniform4fv(glGetUniformLocation(shaderProgramAxisY, "color"), 1, glColAxisY);
    glUseProgram(shaderProgramAxisZ); glUniform4fv(glGetUniformLocation(shaderProgramAxisZ, "color"), 1, glColAxisZ);
    glUseProgram(shaderProgramLimits); glUniform4fv(glGetUniformLocation(shaderProgramLimits, "color"), 1, glColLimits);
    glUseProgram(shaderProgramHighlight); glUniform4fv(glGetUniformLocation(shaderProgramHighlight, "color"), 1, glColToolHighlight);

    for (unsigned int i=0; i<4; i++){
        glUseProgram(*shaderProgramGptr[i]);
        glUniform4fv(glGetUniformLocation(*shaderProgramGptr[i], "color"), 1, glColTools[i]);
        glUniform1ui(glGetUniformLocation(*shaderProgramGptr[i], "clipPlaneYEn"), 0);
        glUniform1f(glGetUniformLocation(*shaderProgramGptr[i], "clipPlaneY"), (clippingYpos + 0.01) * glDrawRatio);
    }

    glUseProgram(shaderProgramHeightMap);
    glUniform1f(glGetUniformLocation(shaderProgramHeightMap, "yMin"), limits->zMinWork * glDrawRatio);
    glUniform1f(glGetUniformLocation(shaderProgramHeightMap, "yMax"), limits->zMaxWork * glDrawRatio);
    glUniform4fv(glGetUniformLocation(shaderProgramHeightMap, "yMinColor"), 1, glColHeightMapMin);
    glUniform4fv(glGetUniformLocation(shaderProgramHeightMap, "yMaxColor"), 1, glColHeightMapMax);
    glUniform4fv(glGetUniformLocation(shaderProgramHeightMap, "crashColor"), 1, glColHeightMapCrash);

    //default settings
    char strCfgBuffer[cfg_vars_arr_size][12];
    char layerDisplayStr[100]; strcpy(layerDisplayStr, strGLCore[STR_GLCORE::GLCORE_LAYER_DISPLAY][language]);
    char layerName [NCPARSER_NAME_LEN + 8];
    bool displayAxis = true, displayGrid = true, displaySubGrid = false, displayLimits = true, displayHeightMap = glHeightMapEnable;
    bool displayExportWindow = false, displayConfigWindow = false, displayAboutWindow = false;
    bool cfgBufferReset = true, sideBarLastElement = false; unsigned int bufferDrawn = 0;
    ImVec2 mainMenuBarSize = ImVec2(0.f, 0.f), /*mainSideBarSize = ImVec2(0.f, 0.f), */mainSliderBarSize = ImVec2(0.f, 0.f);
    int mainMenuStartX = 0;
    double tmpDouble; //store temp data
    const char* toolpathName[5] = {
        strGLCore[STR_GLCORE::GLCORE_TOOL_FAST][language],
        strGLCore[STR_GLCORE::GLCORE_TOOL_WORK][language],
        strGLCore[STR_GLCORE::GLCORE_TOOL_CIRC][language],
        strGLCore[STR_GLCORE::GLCORE_TOOL_DRILL][language],
        strGLCore[STR_GLCORE::GLCORE_TOOL_TOTAL][language],
    };
    char timeArr[32];
    if (layersLimit==0 && strlen(comments[0].name) == 0){strcpy(comments[0].name, strGLCore[STR_GLCORE::GLCORE_NC_NOCOMMENT][language]);} //set default name if no comments
    
    //ImGui colors
    ImVec4 textColorArr[5] = {
        ImVec4(glColTools[0][0], glColTools[0][1], glColTools[0][2], 1.f), //fast
        ImVec4(glColTools[1][0], glColTools[1][1], glColTools[1][2], 1.f), //work
        ImVec4(glColTools[2][0], glColTools[2][1], glColTools[2][2], 1.f), //circular
        ImVec4(glColTools[3][0], glColTools[3][1], glColTools[3][2], 1.f), //drill
        ImVec4(1.f, 1.f, 1.f, 1.f) //total
    };
    ImVec4 textInfoColor = ImVec4(1.f, 1.f, 1.f, 1.f);
    ImVec4 textPercentColor = ImVec4(1.f, 0.5f, 0.1f, 1.f);
    ImVec4 textSideBarColor = ImVec4(.0f, .8f, .8f, 1.f);
    ImVec4 textClippingYColor = ImVec4(0.2f, 1.f, 0.2f, 0.8f);
    ImVec4 textAxisXColor = ImVec4(glColAxisX [0], glColAxisX [1], glColAxisX [2], 1.f);
    ImVec4 textAxisYColor = ImVec4(glColAxisY [0], glColAxisY [1], glColAxisY [2], 1.f);
    ImVec4 textAxisZColor = ImVec4(glColAxisZ [0], glColAxisZ [1], glColAxisZ [2], 1.f);

    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4 (.4f, .4f, .4f, 6.f)); //set lighter color for separator

    //ImGui window/buttons char array
    char exportWindowTitle[strlen(strGLCore[STR_GLCORE::GLCORE_MENU_EXPORT][language]) + 15]; sprintf(exportWindowTitle ,"%s###expwindow", strGLCore[STR_GLCORE::GLCORE_MENU_EXPORT][language]);
    char exportObjSave[strlen(strGLCore[STR_GLCORE::GLCORE_EXPORT_OBJ][language]) + 15]; sprintf(exportObjSave ,"%s###objExport", strGLCore[STR_GLCORE::GLCORE_EXPORT_OBJ][language]);
    char exportStlSave[strlen(strGLCore[STR_GLCORE::GLCORE_EXPORT_STL][language]) + 15]; sprintf(exportStlSave ,"%s###stlExport", strGLCore[STR_GLCORE::GLCORE_EXPORT_STL][language]);
    char exportBtnCancel[strlen(strGLCore[STR_GLCORE::GLCORE_WINDOW_CANCEL][language]) + 15]; sprintf(exportBtnCancel ,"%s###expCancel", strGLCore[STR_GLCORE::GLCORE_WINDOW_CANCEL][language]);

    char cfgWindowTitle[strlen(strGLCore[STR_GLCORE::GLCORE_MENU_CFG][language]) + 15]; sprintf(cfgWindowTitle ,"%s###cfgwindow", strGLCore[STR_GLCORE::GLCORE_MENU_CFG][language]);
    char cfgBtnSave[strlen(strGLCore[STR_GLCORE::GLCORE_WINDOW_SAVE][language]) + 13]; sprintf(cfgBtnSave ,"%s###cfgSave", strGLCore[STR_GLCORE::GLCORE_WINDOW_SAVE][language]);
    char cfgBtnReset[strlen(strGLCore[STR_GLCORE::GLCORE_WINDOW_RESETDEFAULT][language]) + 14]; sprintf(cfgBtnReset ,"%s###cfgReset", strGLCore[STR_GLCORE::GLCORE_WINDOW_RESETDEFAULT][language]);
    char cfgBtnCancel[strlen(strGLCore[STR_GLCORE::GLCORE_WINDOW_CANCEL][language]) + 15]; sprintf(cfgBtnCancel ,"%s###cfgCancel", strGLCore[STR_GLCORE::GLCORE_WINDOW_CANCEL][language]);
    
    char aboutWindowTitle[strlen(strGLCore[STR_GLCORE::GLCORE_MENU_ABOUT][language]) + 17]; sprintf(aboutWindowTitle ,"%s###aboutwindow", strGLCore[STR_GLCORE::GLCORE_MENU_ABOUT][language]);
    char aboutWindowBtnClose[strlen(strGLCore[STR_GLCORE::GLCORE_WINDOW_CLOSE][language]) + 16]; sprintf(aboutWindowBtnClose ,"%s###aboutClose", strGLCore[STR_GLCORE::GLCORE_WINDOW_CLOSE][language]);

    //default conversion factors
    glCamAtDist = (tmpXmax - tmpXmin) * 0.05; //initial camera distance
    float keyboardSlide; //slide value when keyboard used

    //default camera position
    glCamYaw = deg2rad(30);
    glCamYawInit = glCamYaw;
    glCamPitch = deg2rad(45);
    glCamPitchInit = glCamPitch;
    glCamX = glCamAtX + sin(glCamYaw) * cos(glCamPitch) * glCamAtDist;
    glCamZ = glCamAtZ + cos(glCamYaw) * cos(glCamPitch) * glCamAtDist;
    glCamY = glCamAtY + sin(glCamPitch) * glCamAtDist;

    bool glFirstStart = true;
    while (!glfwWindowShouldClose(window)){ //loop until the user closes the window
        if (cfgBufferReset){ //load current config settings, 0.4a ok
            for (unsigned int i = 0; i < cfg_vars_arr_size; i++){config_value_to_str(strCfgBuffer[i], 99, cfg_vars[i].type, cfg_vars[i].ptr);}
            cfgBufferReset = false; //resets for next loop
        }

        glfwPollEvents(); //poll for and process events

        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame(); //init ImGui frame, needed here to get mouse wheel data

        //avoid glfw mouse callback conflict with ImGui
        if (!glImGuiFocused){
            ImGuiMouseDelta = ImGuiCurrentIO.MouseDelta;
            ImGuiMouseWheel = ImGuiCurrentIO.MouseWheel;
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)){glUpdateCamPosition(ImGuiMouseDelta.x, ImGuiMouseDelta.y, 0., 0., 0., false);} //left button pressed
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)){glUpdateCamPosition(0., 0., -ImGuiMouseDelta.x, ImGuiMouseDelta.y, 0., false);} //right button pressed
            if (abs(ImGuiMouseWheel) > 0.001){glUpdateCamPosition(0., 0., 0., 0., ImGuiMouseWheel * glMouseScrollRatio * 15, false);} //scroll
        }

        //avoid glfw keyboard callback conflict with ImGui
        if (ImGui::IsKeyDown(ImGuiKey_RightShift) || ImGui::IsKeyDown(ImGuiKey_LeftShift)){keyboardSlide = 6.f;} else {keyboardSlide = 3.f;} //double slide speed when shift key pressed
        if (ImGui::IsKeyDown(ImGuiKey_UpArrow) || ImGui::IsKeyDown(ImGuiKey_Z)){glUpdateCamPosition(0., 0., 0., 0., -keyboardSlide * glMouseScrollRatio, true);} //up or 'z': forward
        if (ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_Q)){glUpdateCamPosition(0, 0, -keyboardSlide, 0, 0, true);} //left or 'q': slide left
        if (ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_D)){glUpdateCamPosition(0, 0, keyboardSlide, 0, 0, true);} //right or 'd': slide right
        if (ImGui::IsKeyDown(ImGuiKey_DownArrow) || ImGui::IsKeyDown(ImGuiKey_S)){glUpdateCamPosition(0., 0., 0., 0., keyboardSlide * glMouseScrollRatio, true);} //back or 's': backward
        if (ImGui::IsKeyDown(ImGuiKey_Escape)){glfwSetWindowShouldClose(window, 1);} //'esc', quit program

        glfwGetWindowSize(window, &screenWidth, &screenHeight); //get viewport size
        //take menubar and sidebar in account
        //if ((int)abs(screenHeight - mainSideBarSize.y) < 1){
        //    screenWidth -= mainSideBarSize.x;
        //    screenXoff = (int)mainSideBarSize.x;
        //} else {
        //    screenXoff = 0;
        //}
        screenHeight -= mainMenuBarSize.y; screenWidth -= mainSliderBarSize.x; 

        if (screenWidth > 0 && screenHeight > 0 && (screenWidthLast != screenWidth || screenHeightLast != screenHeight)){ //screen size changed
            glViewportUpdate = true; //force update viewport
            glViewport(screenXoff, 0, screenWidth, screenHeight); //update viewport
            debug_stderr("screenWidth:%d, screenHeight:%d\n",screenWidth,screenHeight);

            glProjection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.01f, 100.0f);
            glBindBuffer(GL_UNIFORM_BUFFER, UBOviewportMats);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(glProjection));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glMouseScrollRatio = glCamSlideRatio = (glCamAtDist * atan(deg2rad(90) / 2) * ((float)screenWidth / (float)screenHeight)) / (float)screenWidth; //update scroll and slide ratio
        }

        if (glViewportForceUpdate || glViewportUpdate || bufferDrawn != 3){ //viewport needs update
            if (glViewportUpdate){
                bufferDrawn = 0; glViewportUpdate = false; //toogle update vars
                glUpdateDepth(gridBondaries, &depthNear, &depthFar); //update depth values
            }
            bufferDrawn++;

            //camera/view transformation
            glCamView = glm::lookAt(glm::vec3(glCamX, glCamY, glCamZ), glm::vec3(glCamAtX, glCamAtY, glCamAtZ), glm::vec3(0.0f, 1.0f, 0.0f));
            glBindBuffer(GL_UNIFORM_BUFFER, UBOviewportMats);
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glCamView));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            
            //clear current buffer
            glClearColor(0.3f,0.3f,0.3f,1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (displayLimits){ //draw limits
                glUseProgram(shaderProgramLimits);
                glBindVertexArray(VAOlimits);
                glDrawArrays(GL_LINES, 0, verticesLimitsCount/3);
            }
            
            if (displayAxis){ //draw axis
                glUseProgram(shaderProgramAxisZ); glBindVertexArray(VAOaxisZ); glDrawArrays(GL_LINE_STRIP, 0, verticesAxisCount/3); //z axis
                glUseProgram(shaderProgramAxisX); glBindVertexArray(VAOaxisX); glDrawArrays(GL_LINE_STRIP, 0, verticesAxisCount/3); //x axis
                glUseProgram(shaderProgramAxisY); glBindVertexArray(VAOaxisY); glDrawArrays(GL_LINE_STRIP, 0, verticesAxisCount/3); //y axis
            }

            if (glHeightMapEnable && displayHeightMap){ //draw heightmap
                glUseProgram(shaderProgramHeightMap);
                glUniform3f(glGetUniformLocation(shaderProgramHeightMap, "camPosition"), glCamX, glCamY, glCamZ);
                glBindVertexArray(VAOHeightMap);
                glDrawArrays(GL_TRIANGLES, 0, verticesHeightMapCount / 6);
            }

            //draw layers
            layersEnabled = 0; //reset layer enable count (ImGui menu)
            for (unsigned int i=0; i<layersLimit; i++){
                if (layersEnable[i]){ //layer is enabled
                    layersEnabled++;
                    for (unsigned int j=0; j<4; j++){
                        if (layersOpEnable[j]){ //operation enabled
                            tmpShaderProgram = (layersHovered[i]) ? shaderProgramHighlight : *shaderProgramGptr[j];
                            glUseProgram(tmpShaderProgram);
                            glUniform3f(glGetUniformLocation(tmpShaderProgram, "camPosition"), glCamX, glCamY, glCamZ);
                            glUniform1f(glGetUniformLocation(tmpShaderProgram, "near"), depthNear);
                            glUniform1f(glGetUniformLocation(tmpShaderProgram, "far"), depthFar);
                            glBindVertexArray(layersVAO[i][j]);
                            glDrawArrays(GL_LINES, 0, layersVertCount[i][j] / 3);
                        }
                    }
                }
            }

            if (displaySubGrid){ //draw subgrid
                glUseProgram(shaderProgramSubGrid);
                glBindVertexArray(VAOsubGrid);
                glDrawArrays(GL_LINES, 0, verticesSubGridCount/3);
            }

            if (displayGrid){ //draw grid
                glUseProgram(shaderProgramGrid);
                glBindVertexArray(VAOgrid);
                glDrawArrays(GL_LINES, 0, verticesGridCount/3);
            }
        }


        //GUI
        //start menubar
        if (ImGui::BeginMainMenuBar()){
            mainMenuBarSize = ImGui::GetWindowSize(); //backup menu height for side tab positioning
            ImGui::SetCursorPosX(mainMenuStartX); //offset to avoid menu collide with side menu

            ImGui::Separator();
            if (ImGui::BeginMenu(strGLCore[STR_GLCORE::GLCORE_MENU_TOOLPATH][language])){ //toolpaths menu
                ImGui::PushStyleColor(ImGuiCol_Text, textColorArr[0]); ImGui::MenuItem(toolpathName[0], NULL, &layersOpEnable[0]); ImGui::PopStyleColor();
                ImGui::PushStyleColor(ImGuiCol_Text, textColorArr[1]); ImGui::MenuItem(toolpathName[1], NULL, &layersOpEnable[1]); ImGui::PopStyleColor();
                ImGui::PushStyleColor(ImGuiCol_Text, textColorArr[2]); ImGui::MenuItem(toolpathName[2], NULL, &layersOpEnable[2]); ImGui::PopStyleColor();
                ImGui::PushStyleColor(ImGuiCol_Text, textColorArr[3]); ImGui::MenuItem(toolpathName[3], NULL, &layersOpEnable[3]); ImGui::PopStyleColor();
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            if(ImGui::BeginMenu(strGLCore[STR_GLCORE::GLCORE_MENU_DISPLAY][language])){ //display menu
                if (ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_MENU_RESETVIEW][language], NULL, false)){
                    glCamYaw = glCamYawInit; glCamPitch = glCamPitchInit; glCamAtX = glCamAtXinit; glCamAtY = glCamAtYinit; glCamAtZ = glCamAtZinit;
                    glUpdateCamPosition (0., 0., 0., 0., 0., true);
                }
                ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_MENU_AXIS][language], NULL, &displayAxis);
                ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_MENU_GRID][language], NULL, &displayGrid);
                ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_MENU_SUBGRID][language], NULL, &displaySubGrid);
                ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_MENU_LIMITS][language], NULL, &displayLimits);
                if (glHeightMapEnable){ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_MENU_HEIGHTMAP][language], NULL, &displayHeightMap);}
                if (ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_LAYER_LAYERS][language], NULL, layersEnabled != 0)){for (unsigned int i=0; i<layersLimit; i++){layersEnable[i]=(layersEnabled==0)?true:false;}}
                ImGui::EndMenu();
            }

            if (glHeightMapEnable){
                ImGui::Separator();
                if(ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_MENU_EXPORT][language], NULL, false)){displayExportWindow = true;} //export menu
            }

            ImGui::Separator();
            if(ImGui::BeginMenu(strGLCore[STR_GLCORE::GLCORE_MENU_SETTINGS][language])){ //settings menu
                if (ImGui::BeginMenu(strGLCore[STR_GLCORE::GLCORE_MENU_DISPLAY][language])){ //settings menu
                    if (ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_MENU_WIREFRAME][language], NULL, &glWireframeEnabled)){(glWireframeEnabled) ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); glViewportUpdate = true;}
                    if (ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_MENU_AA][language], NULL, &glAAenabled)){(glAAenabled) ? glEnable(GL_LINE_SMOOTH) : glDisable(GL_LINE_SMOOTH); glViewportUpdate = true;}
                    ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_MENU_LIMITFPS][language], NULL, &glVsyncEnabled);
                    ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_MENU_FORCEUPD][language], NULL, &glViewportForceUpdate);
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_MENU_CFG][language], NULL, false)){displayConfigWindow = true;}
                if (ImGui::MenuItem(strGLCore[STR_GLCORE::GLCORE_MENU_ABOUT][language], NULL, false)){displayAboutWindow = true;}
                ImGui::EndMenu();
            }

            ImGui::Separator();
            if (glFirstStart){mainMenuStartX = (int)ImGui::GetCursorPos().x + (int)ImGui::GetStyle().ItemSpacing.x * 2;}

            //draw Z clipping value
            ImGui::SetCursorPos(ImVec2(0.f, -100.f)); ImGui::Text(clippingYstr); //draw offscreen to get proper item size
            ImVec2 clippingTextSize = ImGui::GetItemRectSize(); ImGui::SetCursorPos(ImVec2(screenXoff + screenWidth - clippingTextSize.x + 5.f, 0.f));
            ImGui::TextColored(textClippingYColor, clippingYstr);
            if (glFirstStart){
                char tmpStr[strlen(strGLCore[STR_GLCORE::GLCORE_TOOL_ZCLIPPING][language]) + 20];
                sprintf(tmpStr, " %s: -%.2lfmm", strGLCore[STR_GLCORE::GLCORE_TOOL_ZCLIPPING][language], abs(limits->zMin) + abs(limits->zMax));
                ImVec2 tmpSize = ImGui::CalcTextSize(tmpStr, nullptr, false, 0.f);
                mainMenuStartX += (int)tmpSize.x + 5;
            }
            ImGui::EndMainMenuBar();
        }

        //start sidebar
        ImGui::SetNextWindowPos(ImVec2 (0.f,0.f)); ImGui::SetNextWindowSize(ImVec2 (0.f,(float)screenHeight + mainMenuBarSize.y)); //set sidebar position and size
        ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once); //collapse report menu
        if (ImGui::Begin(strGLCore[STR_GLCORE::GLCORE_LAYER_REPORT][language], NULL, sideBarFlags)){
            //mainSideBarSize = ImGui::GetWindowSize(); //backup sidebar sizes for menu positioning
            if (glFirstStart){ImGui::BeginGroup();}
            for (unsigned int i = 0; i < layersLimit + 1; i++){ //layers loop
                if (i < layersLimit){
                    tmpDouble = comments[i].distTotal;
                    sideBarLastElement = false; //layer
                } else { //total
                    tmpDouble = summary->distTotal;
                    sideBarLastElement = true;
                }
                if (tmpDouble > 0.0001){ //something happened in current layer
                    if (sideBarLastElement){
                        ImGui::Separator();
                        sprintf(layerName, "%s  ###%d", strGLCore[STR_GLCORE::GLCORE_TOOL_TOTAL][language], i); //total menu name
                    } else {
                        sprintf(layerName, "%s  ###%d", comments[i].name,i); //layers menu name
                    }
                    if (ImGui::TreeNodeEx(layerName, (sideBarLastElement ? ImGuiTreeNodeFlags_DefaultOpen : (layersEnable[i]) ? ImGuiTreeNodeFlags_Selected : 0))){ //open total by default
                        if (!sideBarLastElement && ImGui::IsItemHovered()){ //menu is howered, also avoid a glitch where menu disappair without notice
                            layersHovered [i] = true;
                            glViewportUpdate = true;
                        } else {
                            layersHovered [i] = false;
                        }
                        if (!sideBarLastElement){
                            sprintf(layerDisplayStr, "%s###dispLayer%d", (layersEnable[i]) ? strGLCore[STR_GLCORE::GLCORE_LAYER_HIDE][language] : strGLCore[STR_GLCORE::GLCORE_LAYER_DISPLAY][language], i); //layers menu name
                            ImGui::SameLine(); if(ImGui::SmallButton(layerDisplayStr)){layersEnable[i] = !layersEnable[i];}
                        }
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 2.f)); //disable item spacing
                        ImGui::PushStyleColor(ImGuiCol_Text, textSideBarColor); //custom text color

                        double opsDistArr[5] = { //total : layer
                            sideBarLastElement ? summary->distFastTotal : comments[i].distFast,
                            sideBarLastElement ? summary->distWorkTotal : comments[i].distWork,
                            sideBarLastElement ? summary->distCircularTotal : comments[i].distCircular,
                            sideBarLastElement ? summary->distDrillTotal : comments[i].distDrill,
                            sideBarLastElement ? summary->distTotal : comments[i].distTotal,
                        };

                        double opsTimeArr[5] = { //total : layer
                            sideBarLastElement ? summary->timeFastTotal : comments[i].timeFast,
                            sideBarLastElement ? summary->timeWorkTotal : comments[i].timeWork,
                            sideBarLastElement ? summary->timeCircularTotal : comments[i].timeCircular,
                            sideBarLastElement ? summary->timeDrillTotal : comments[i].timeDrill,
                            sideBarLastElement ? summary->timeTotal : comments[i].timeTotal,
                        };

                        for (unsigned int j = 0; j < 5; j++){ //toolpaths, 0.4a ok
                            if (opsDistArr[j] > 0.0001){ //dist > 0
                                sec2charArr(timeArr, opsTimeArr[j] * 60);
                                ImGui::TextColored(textColorArr[j], toolpathName[j]);
                                ImGui::SameLine(); ImGui::Text(": "); ImGui::SameLine(); ImGui::TextColored(textInfoColor, "%.2lfmm", opsDistArr[j]);
                                ImGui::SameLine(); ImGui::Text(", "); ImGui::SameLine(); ImGui::TextColored(textInfoColor, "%s", timeArr);
                                if (speedPercent != 100){
                                    sec2charArr (timeArr, (opsTimeArr[j] / ((double)speedPercent / 100)) * 60);
                                    ImGui::SameLine(); ImGui::Text(", "); ImGui::SameLine(); ImGui::TextColored(textPercentColor, "%d%%", speedPercent);
                                    ImGui::SameLine(); ImGui::Text(": "); ImGui::SameLine(); ImGui::TextColored(textInfoColor, "%s", timeArr);
                                }
                            }
                        }
                        
                        ImGui::Separator(); //add separator at the end
                        ImGui::PopStyleVar(); ImGui::PopStyleColor(); //restore initial style
                        ImGui::TreePop();
                    } else {
                        if (!sideBarLastElement && ImGui::IsItemHovered()){layersHovered[i] = true; glViewportUpdate = true;} else {layersHovered[i] = false;} //menu is howered
                    }
                }
            }

            ImGui::Separator();
            if (ImGui::TreeNodeEx(strGLCore[STR_GLCORE::GLCORE_MENU_LIMITS][language], (displayLimits) ? ImGuiTreeNodeFlags_Selected : 0)){ //limits
                sprintf(layerDisplayStr, "%s###dispLimits", (displayLimits) ? strGLCore[STR_GLCORE::GLCORE_LAYER_HIDE][language] : strGLCore[STR_GLCORE::GLCORE_LAYER_DISPLAY][language]);
                ImGui::SameLine(); if (ImGui::SmallButton(layerDisplayStr)){displayLimits = !displayLimits;}
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 2.f)); //disable item spacing
                ImGui::TextColored(textAxisXColor, "X: "); ImGui::SameLine(); ImGui::Text("%.2lfmm <> %.2lfmm", limits->xMin, limits->xMax);
                ImGui::TextColored(textAxisYColor, "Y: "); ImGui::SameLine(); ImGui::Text("%.2lfmm <> %.2lfmm", limits->yMin, limits->yMax);
                ImGui::TextColored(textAxisZColor, "Z: "); ImGui::SameLine(); ImGui::Text("%.2lfmm <> %.2lfmm", limits->zMin, limits->zMax);
                ImGui::PopStyleVar();
                ImGui::TreePop();
            }

            //resize gl app window to fix menubar menus overflow if a layer name too long
            if (glFirstStart){
                ImGui::EndGroup();
                screenWidth = (int)ImGui::GetItemRectSize().x + mainMenuStartX;
                if (screenWidth > (int)glViewportWidth){
                    glfwSetWindowSize(window, screenWidth, glViewportHeight);
                    mainMenuStartX = screenWidth - mainMenuStartX;
                } else {mainMenuStartX = glViewportWidth - mainMenuStartX;}
                mainMenuStartX += (int)ImGui::GetStyle().ItemSpacing.x * 2;
                glFirstStart = false;
            }

            ImGui::End();
        }

        //start sliderbar
        ImGui::SetNextWindowPos(ImVec2 (screenXoff + screenWidth, mainMenuBarSize.y)); ImGui::SetNextWindowSize(ImVec2 (12.f, (float)screenHeight)); //set position and size
        if (ImGui::Begin("###sliderwindow", NULL, sliderBarFlags)){
            mainSliderBarSize = ImGui::GetWindowSize(); //backup sliderbar sizes for menu positioning
            ImGui::SetCursorPos (ImVec2 (0.f, 0.f));
            if (ImGui::VSliderFloat("###YclippingSlider", ImVec2 (10.f, screenHeight), &clippingYpos, (float)limits->zMin, (float)limits->zMax, "", ImGuiSliderFlags_NoInput)){
                sprintf(clippingYstr, "%s: %.2lfmm", strGLCore[STR_GLCORE::GLCORE_TOOL_ZCLIPPING][language], clippingYpos);
                for (unsigned int j=0; j<4; j++){ //operation loop
                    glUseProgram(*shaderProgramGptr[j]);
                    glUniform1ui(glGetUniformLocation(*shaderProgramGptr[j], "clipPlaneYEn"), (limits->zMax - clippingYpos < 0.001) ? 0 : 1);
                    glUniform1f(glGetUniformLocation(*shaderProgramGptr[j], "clipPlaneY"), (clippingYpos + 0.01) * glDrawRatio);
                }
            }
            ImGui::End();
        }

        //display export window
        if (displayExportWindow){
            if (ImGui::Begin(exportWindowTitle, NULL, configWindowFlags)){ //start ImGui frame
                LONG_TYPE tmpVerticesHeightMapCount = verticesHeightMapCount / 6; //6 because each points coods also have normals vertors
                int numLength = strNumSepFormatingLength(tmpVerticesHeightMapCount);
                char tmpStr[numLength];
                strNumSepFormating(tmpStr, numLength, ' ', tmpVerticesHeightMapCount);
                ImGui::Text("%s: %s", strGLCore[STR_GLCORE::GLCORE_EXPORT_VERTICES_COUNT][language], tmpStr);

                LONG_TYPE trianglesHeightMapCount = tmpVerticesHeightMapCount / 3;
                numLength = strNumSepFormatingLength(trianglesHeightMapCount);
                strNumSepFormating(tmpStr, numLength, ' ', trianglesHeightMapCount);
                ImGui::Text("%s: %s", strGLCore[STR_GLCORE::GLCORE_EXPORT_TRIANGLES_COUNT][language], tmpStr);

                ImGui::Separator();

                if (ImGui::Button (exportStlSave)){
                    char exportFilePath[PATH_MAX]; sprintf(exportFilePath, "%s.heightmap.stl", file);
                    glExportSTLbin(exportFilePath, verticesHeightMapPtr, 6, verticesHeightMapCount / (6 * 3), 0, -heightMapZoffset, 0, 1 / glDrawRatio); //export vertices array to binary stl file
                    displayExportWindow = false;
                }

                ImGui::SameLine ();
                if (ImGui::Button (exportObjSave)){
                    char exportFilePath[PATH_MAX]; sprintf(exportFilePath, "%s.heightmap.obj", file);
                    glExportOBJ(exportFilePath, verticesHeightMapPtr, 6, verticesHeightMapCount / (6 * 3), 0, -heightMapZoffset, 0, 1 / glDrawRatio); //export vertices array to obj file
                    displayExportWindow = false;
                }

                ImGui::SameLine();
                if (ImGui::Button (cfgBtnCancel)){displayExportWindow = false;}
                ImGui::End();
            }
        }

        //display config window
        if (displayConfigWindow){
            if (ImGui::Begin(cfgWindowTitle, NULL, configWindowFlags)){ //start ImGui frame
                char tmpVarsID[14];
                for (unsigned int i = 0; i < cfg_vars_arr_size; i++){
                    sprintf(tmpVarsID, "###cgrInp%d", i); //generate InputText id
                    char* tmpPtr = (char*)cfg_vars[i].name;
                    if (strstr(tmpPtr, TXTNL)){
                        ImGui::Separator();
                        tmpPtr += strlen(TXTNL);
                    }
                    ImGui::Text("%s: ", tmpPtr);
                    ImGui::SameLine();
                    ImGui::InputText(tmpVarsID, strCfgBuffer[i], 11, ImGuiInputTextFlags_CharsDecimal);
                }
                ImGui::Separator();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.9f, 0.f, 0.f, 1.0f)); ImGui::Text("%s", strGLCore[STR_GLCORE::GLCORE_CFG_APPLYWARN][language]); ImGui::PopStyleColor();
                ImGui::Separator();
                if (ImGui::Button (cfgBtnSave)){
                    for (unsigned int i = 0; i < cfg_vars_arr_size; i++){
                        config_type_parse(cfg_vars, cfg_vars_arr_size, i, cfg_vars[i].type, (char*)cfg_vars[i].name, strCfgBuffer[i]);
                    }
                    config_save(cfg_vars, cfg_vars_arr_size, (char*)cfg_filename, -1, -1, false); //save config file
                    displayConfigWindow = false;
                }

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 0.5f, 0.1f, 1.f));
                ImGui::SameLine();
                if(ImGui::Button (cfgBtnReset)){cfgBufferReset = true;}
                ImGui::PopStyleColor();

                ImGui::SameLine();
                if(ImGui::Button (cfgBtnCancel)){displayConfigWindow = false;}
                ImGui::End();
            }
        }

        //display about window
        if (displayAboutWindow){
            if (ImGui::Begin(aboutWindowTitle, NULL, configWindowFlags)){ //start ImGui frame
                ImGui::Text ("%s v%s: %s", "nc2png", programversion, "https://github.com/porcinus/nc2png");
                ImGui::Separator();
                ImGui::Text ("%s:", strGLCore[STR_GLCORE::GLCORE_ABOUT_EXTLIB][language]);
                ImGui::Text ("\tlibGD (%s): https://libgd.github.io/", GD_VERSION_STRING);
                ImGui::Text ("\tlibpng (%s): http://www.libpng.org/", PNG_LIBPNG_VER_STRING);
                ImGui::Text ("\tzlib (%s): https://zlib.net/", ZLIB_VERSION);
                ImGui::Separator();
                ImGui::Text ("%s:", "OpenGL");
                ImGui::Text ("\tGLFW (%d.%d.%d): https://www.glfw.org/", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
                ImGui::Text ("\tGLAD: https://glad.dav1d.de/");
                ImGui::Text ("\tGLM: https://github.com/g-truc/glm");
                ImGui::Text ("\tImGui (%s): https://github.com/ocornut/imgui", ImGui::GetVersion());
                ImGui::Text ("\tHuge thanks: https://learnopengl.com/Introduction");
                ImGui::Separator();
                if (ImGui::Button(aboutWindowBtnClose)){displayAboutWindow = false;}
                ImGui::End();
            }
        }


        //render
        glImGuiFocused = ImGui::IsAnyItemHovered() || ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow); //any ImGui instance hovered
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)){ImGui::SetWindowFocus(NULL);} //reset focus
        ImGui::Render(); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); //render ImGui frame

        glfwSwapBuffers(window); //swap buffers

        gettimeofday(&tv, NULL); frameEndTV = 1000000 * tv.tv_sec + tv.tv_usec; //frame end time
        if (glVsyncEnabled){ //pseudo vsync, gdfw vsync drain a full core for nothing
            frameSleep = 16666.66666666667 - (frameEndTV - frameStartTV);
            if (frameSleep > 0){usleep(frameSleep);} //sleep
            gettimeofday(&tv, NULL); frameStartTV = 1000000 * tv.tv_sec + tv.tv_usec; //frame start time
        }

        if (frameEndTV - glViewportLastUpdate > 100000){glViewportUpdate = true; glViewportLastUpdate = frameEndTV;} //limit viewport update to 10fps when nothing happen
        if (frameEndTV - glFPSlastTime > 1000000){ //1sec
            char windowTitleFin[windowTitleSize];
            snprintf(windowTitleFin, windowTitleSize - 1, "%s (%dfps)", windowTitle, glFPS);
            glfwSetWindowTitle(window, windowTitleFin); //update window title
            glFPS = 0; glFPSlastTime = frameEndTV; //reset fps counter and backup last time for fps counter
        } else {glFPS++;} //increment fps counter

        screenWidthLast = screenWidth, screenHeightLast = screenHeight; //backup screen size
    }

    if(debugOutput){printf("GLFW window closing\n");}
    
    //ImGui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glProgramClose();
    debug = debugBack;

    return 0;
}

