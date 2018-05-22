#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#define __FRAGMENT__
#include "VertexToFragment.glsl"

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outSecondary;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    outColor = texture(texSampler, fragTexCoord);
    outSecondary = texture(texSampler, fragTexCoord).bgra;
}