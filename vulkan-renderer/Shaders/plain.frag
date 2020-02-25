#version 450
#extension GL_ARB_separate_shader_objects : enable

// varyings
#if IN_NORMAL
layout(location = 5) in mat3 inTBN;
#endif
layout(location = 0) in vec3 inPosition;

#if IN_NORMAL
layout(location = IN_NORMAL_LOCATION) in vec3 inNormal;
#endif

#if IN_TANGENT
layout(location = IN_TANGENT_LOCATION) in vec3 inTangent;
#endif

#if IN_UV0
layout(location = IN_UV0_LOCATION) in vec3 inUv;
#endif

#if IN_COLOR
layout(location = IN_COLOR_LOCATION) in vec3 inColor;
#endif

//output
layout(location = 0) out vec4 outColor;

void main() 
{
#if IN_COLOR
    outColor = vec4(inColor, 1.0);
#endif
}