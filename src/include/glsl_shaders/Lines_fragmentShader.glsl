// NC2PNG
// Generate preview and time estimations based on gcode file (milling)
// 
// Basic lines fragment shader
// 
// Editing this file doesn't affect internal program GL shaders.
// If you plan to recompile program, please edit ./include/glcore_shaders.h accordingly

#version 330 core
uniform vec4 color = vec4 (1.f,1.f,1.f,1.f);
out vec4 finalFragColor;
void main(){finalFragColor = color;}