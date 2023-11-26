/*
NC2PNG
Generate preview and time estimations based on gcode file (milling)

Related to OpenGl window.

Important notes:
- If you update ImGui, please edit imgui_widgets.cpp and add in ImGui::BeginMainMenuBar() window_flags : " | ImGuiWindowFlags_NoBringToFrontOnFocus".
- If you happen to update shaders from "glsl_shaders" folder, included shaders file (e.g. this file) won't be updated, so you will need to update it yourself.
*/

#ifndef GLCORE_SHADERS_H
#define GLCORE_SHADERS_H

//generic lines shader
const char *vertexShaderLinesStr = //external : glsl_shaders/Lines_vertexShader.glsl
"#version 330 core\n"
"layout (location = 0) in vec3 vertexPosition;"
"layout (std140) uniform viewportMats {mat4 projection; mat4 view;};"
"void main(){gl_Position = projection * view * vec4(vertexPosition, 1.0f);}\0";

const char *fragmentShaderLinesStr = //external : glsl_shaders/Lines_fragmentShader.glsl
"#version 330 core\n"
"uniform vec4 color = vec4 (1.f, 1.f, 1.f, 1.f);"
"out vec4 finalFragColor;"
"void main(){finalFragColor = color;}\0";


//tool lines shader
const char *vertexShaderToolLinesStr = //external : glsl_shaders/Lines_vertexShader.glsl
"#version 330 core\n"
"layout (location = 0) in vec3 vertexPosition;"
"layout (std140) uniform viewportMats {mat4 projection; mat4 view;};"
"uniform vec3 camPosition = vec3 (0.f, 0.f, 0.f);"
"uniform float camFar = 0.f, camNear = 0.f, clipPlaneY = 0.f;"
"uniform bool clipPlaneYEn = false;"
"out float fragDepth; flat out int fragYclip;"
"void main(){"
"   gl_Position = projection * view * vec4(vertexPosition, 1.0f);"
"   if (clipPlaneYEn && vertexPosition.y > clipPlaneY){"
"       fragYclip = 1; fragDepth = 1.;"
"   } else {"
"       if ((abs(camFar) + abs(camNear)) > 0.f){"
"           fragDepth = clamp(1 - (distance(camPosition, vertexPosition) - camNear) * (1 / (camFar - camNear)) * 0.5f, 0.001f, 1.f);"
"       } else {"
"           fragDepth = 0.f;"
"       }"
"       fragYclip = 0;"
"   }"
"}\0";

const char *fragmentShaderToolLinesStr = //external : glsl_shaders/ToolLines_fragmentShader.glsl
"#version 330 core\n"
"uniform vec4 color = vec4 (1.f, 1.f, 1.f, 1.f);"
"flat in int fragYclip;"
"in float fragDepth;"
"out vec4 finalFragColor;"
"void main(){"
"   if (fragYclip > 0){discard;}"
"   vec4 tmpColor = color;"
"   if (fragDepth > 0.f){tmpColor.xyz *= fragDepth;}"
"   finalFragColor = tmpColor;"
"}\0";


//heightmap shader
const char *vertexShaderHeightMapStr = //external : glsl_shaders/ToolLines_vertexShader.glsl
"#version 330 core\n"
"layout (location = 0) in vec3 vertexPosition;"
"layout (location = 1) in vec3 normalVector;"
"layout (std140) uniform viewportMats {mat4 projection; mat4 view;};"
"uniform float yMin, yMax;"
"uniform vec4 yMinColor, yMaxColor;"
"uniform vec3 camPosition = vec3(0.f, 0.f, 0.f);"
"out vec4 fragColor;"
"out vec3 fragNormal, fragPos, fragCamPos;"
"void main(){"
"   gl_Position = projection * view * vec4(vertexPosition, 1.0f);"
"   fragColor = yMinColor + (yMaxColor - yMinColor) * (vertexPosition.y - yMin) / (yMax - yMin);"
"   fragPos = vec3(view * vec4(vertexPosition, 1.0f)); "
"   fragNormal = mat3(transpose(inverse(view))) * normalVector;"
"   fragCamPos = vec3(view * vec4(camPosition, 1.0));"
"}\0";

const char *fragmentShaderHeightMapStr = //external : glsl_shaders/ToolLines_fragmentShader.glsl
"#version 330 core\n"
"layout (std140) uniform viewportMats {mat4 projection; mat4 view;};"
"uniform vec4 yMinColor, crashColor;"
"in vec4 fragColor;"
"in vec3 fragNormal, fragPos, fragCamPos;"
"const float COLOR_ROUND_ERROR = 0.01f;" //try to deal with rounding error
"const float COLOR_DIFFUSE_CORR = 0.2f;" //diffuse correction relative to cam position 
"out vec4 finalFragColor;"
"void main(){"
"    vec3 colorMid = fragColor.xyz - yMinColor.xyz;" //select initial fragment color
"    if ((colorMid.x + colorMid.y + colorMid.z) / 3.f < -COLOR_ROUND_ERROR){finalFragColor = crashColor;} else {finalFragColor = fragColor;}" //cheap solution without computing hls
"    vec3 properNormal = fragNormal;" //allow both front and back face lighting
"    if (gl_FrontFacing){properNormal = properNormal * -1;}"
"    properNormal = normalize(properNormal);"
"    vec3 lightDir = normalize(fragCamPos - fragPos);" //final color
"    vec3 diffuseColor = vec3(1.f, 1.f, 1.f) - max(dot(properNormal, lightDir), 0.0) * COLOR_DIFFUSE_CORR;"
"    finalFragColor = vec4(diffuseColor * finalFragColor.xyz, 1.0);"
"}\0";


#endif