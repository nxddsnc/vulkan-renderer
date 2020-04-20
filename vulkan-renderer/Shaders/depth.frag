#version 450

layout(location = 0) in vec3 inPosition;

//output
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
    vec3 cameraPos;
} ubo;

void main()
{
    outColor.r = inPosition.z;
}