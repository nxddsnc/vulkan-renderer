#version 450
#extension GL_ARB_separate_shader_objects : enable

// varyings
layout(location = 0) in mat3 inTBN;
layout(location = 3) in vec3 inPosition;
layout(location = 4) in vec3 inNormal;

#if IN_TANGENT
layout(location = IN_TANGENT_LOCATION) in vec3 inTangent;
#endif

#if IN_UV0
layout(location = IN_UV0_LOCATION + 3) in vec3 inUv;
#endif

#if IN_COLOR
layout(location = IN_COLOR_LOCATION + 3) in vec3 inColor;
#endif

//output
layout(location = 0) out vec4 outColor;

void main() 
{
#if IN_COLOR
    outColor = vec4(inColor, 1.0);
#endif
}