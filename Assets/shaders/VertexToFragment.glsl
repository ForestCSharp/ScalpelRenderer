#ifdef __VERTEX__
#define IN_OUT out
#endif

#ifdef __FRAGMENT__
#define IN_OUT in
#endif

layout(location = 0) IN_OUT vec3 fragColor;
layout(location = 1) IN_OUT vec2 fragTexCoord;