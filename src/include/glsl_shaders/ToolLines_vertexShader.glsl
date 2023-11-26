// NC2PNG
// Generate preview and time estimations based on gcode file (milling)
// 
// Toolpaths (fast, work, circular, drill) vertex shader
// 
// Editing this file doesn't affect internal program GL shaders.
// If you plan to recompile program, please edit ./include/glcore_shaders.h accordingly

#version 330 core
layout (location = 0) in vec3 vertexPosition;
layout (std140) uniform viewportMats {mat4 projection; mat4 view;};
uniform vec3 camPosition = vec3 (0.f, 0.f, 0.f);
uniform float camFar = 0.f, camNear = 0.f, clipPlaneY = 0.f;
uniform bool clipPlaneYEn = false;
out float fragDepth; flat out int fragYclip;
void main(){
   gl_Position = projection * view * vec4(vertexPosition, 1.0f);
   if (clipPlaneYEn && vertexPosition.y > clipPlaneY){
       fragYclip = 1; fragDepth = 1.;
   } else {
       if ((abs(camFar) + abs(camNear)) > 0.f){
           fragDepth = clamp(1 - (distance(camPosition, vertexPosition) - camNear) * (1 / (camFar - camNear)) * 0.5f, 0.001f, 1.f);
       } else {
           fragDepth = 0.f;
       }
       fragYclip = 0;
   }
};