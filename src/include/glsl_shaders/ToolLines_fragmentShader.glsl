// NC2PNG
// Generate preview and time estimations based on gcode file (milling)
// 
// Toolpaths (fast, work, circular, drill) fragment shader
// 
// Editing this file doesn't affect internal program GL shaders.
// If you plan to recompile program, please edit ./include/glcore_shaders.h accordingly

#version 330 core
uniform vec4 color = vec4 (1.f,1.f,1.f,1.f);
flat in int fragYclip;
in float fragDepth;
out vec4 finalFragColor;
void main(){
   if (fragYclip > 0){discard;}
   vec4 tmpColor = color;
   if (fragDepth > 0.f){tmpColor.xyz *= fragDepth;}
   finalFragColor = tmpColor;
};