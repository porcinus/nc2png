// NC2PNG
// Generate preview and time estimations based on gcode file (milling)
// 
// Basic lines vertex shader
// 
// Editing this file doesn't affect internal program GL shaders.
// If you plan to recompile program, please edit ./include/glcore_shaders.h accordingly

#version 330 core
layout (location = 0) in vec3 vertexPosition;
layout (std140) uniform viewportMats {mat4 projection; mat4 view;};
void main(){
    gl_Position = projection * view * vec4(vertexPosition, 1.0f);
}