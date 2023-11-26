// NC2PNG
// Generate preview and time estimations based on gcode file (milling)
// 
// Heightmap vertex shader
// 
// Editing this file doesn't affect internal program GL shaders.
// If you plan to recompile program, please edit ./include/glcore_shaders.h accordingly

#version 330 core
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 normalVector;
layout (std140) uniform viewportMats {mat4 projection; mat4 view;};
uniform float yMin, yMax;
uniform vec4 yMinColor, yMaxColor;
uniform vec3 camPosition = vec3(0.f, 0.f, 0.f);
out vec4 fragColor;
out vec3 fragNormal, fragPos, fragCamPos;
void main(){
   gl_Position = projection * view * vec4(vertexPosition, 1.0f);
   fragColor = yMinColor + (yMaxColor - yMinColor) * (vertexPosition.y - yMin) / (yMax - yMin);
   fragPos = vec3(view * vec4(vertexPosition, 1.0f));
   fragNormal = mat3(transpose(inverse(view))) * normalVector;
   fragCamPos = vec3(view * vec4(camPosition, 1.0));
};