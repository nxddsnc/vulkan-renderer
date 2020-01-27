#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
} ubo;

layout(push_constant) uniform UniformPerDrawable
{
    mat4 modelMatrix;
} uniformPerDrawable;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

#if IN_UV0
layout(location = IN_UV0_LOCATION) in vec2 inUv;
#endif

#if IN_TANGENT
layout(location = IN_TANGENT_LOCATION) in vec3 inTangent;
#endif


// varyings
layout(location = 0) out vec3 outNormal;

#if IN_UV0
layout(location = IN_UV0_LOCATION) out vec2 outUv;
#endif

void main() {
    gl_Position = ubo.proj * ubo.view * uniformPerDrawable.modelMatrix * vec4(inPosition, 1.0);
    outNormal = inNormal;
#if IN_UV0
    outUv = inUv;
#endif
}