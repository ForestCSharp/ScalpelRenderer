#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

//MVP
#include "MVP.glsl"

//Vertex Input Definition
#include "VertexInput.glsl"

//Data to pass to Fragment Shader
#define __VERTEX__
#include "VertexToFragment.glsl"

void main() {
    gl_Position = MVP.proj * MVP.view * MVP.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord * 3;
}