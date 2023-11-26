// NC2PNG
// Generate preview and time estimations based on gcode file (milling)
// 
// Heightmap fragment shader
// 
// Editing this file doesn't affect internal program GL shaders.
// If you plan to recompile program, please edit ./include/glcore_shaders.h accordingly

#version 330 core
layout (std140) uniform viewportMats {mat4 projection; mat4 view;};
uniform float yMin, yMax;
uniform vec4 yMinColor, crashColor;
in vec4 fragColor;
in vec3 fragNormal, fragPos, fragCamPos;
const float COLOR_ROUND_ERROR = 0.01f; //try to deal with rounding error
const float COLOR_DIFFUSE_CORR = 0.2f; //diffuse correction relative to cam position 

out vec4 finalFragColor;
void main(){
    //select initial fragment color
    vec3 colorMid = fragColor.xyz - yMinColor.xyz;
    if ((colorMid.x + colorMid.y + colorMid.z) / 3.f < -COLOR_ROUND_ERROR){finalFragColor = crashColor;} else {finalFragColor = fragColor;}

    //allow both front and back face lighting
    vec3 properNormal = fragNormal;
    if (gl_FrontFacing){properNormal = properNormal * -1;}
    properNormal = normalize(properNormal);

    //final color
    vec3 lightDir = normalize(fragCamPos - fragPos);
    vec3 diffuseColor = vec3(1.f, 1.f, 1.f) - max(dot(properNormal, lightDir), 0.0) * COLOR_DIFFUSE_CORR;
    finalFragColor = vec4(diffuseColor * finalFragColor.xyz, 1.0);
};