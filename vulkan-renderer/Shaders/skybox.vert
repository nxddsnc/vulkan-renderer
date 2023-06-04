#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
} ubo;

layout(location = 0) in vec3 inPosition;

// varyings
layout(location = 0) out vec3 outUVW;

void main() 
{
    outUVW = vec3(inPosition.x, inPosition.y, inPosition.z).xyz;
    mat3 view = mat3(ubo.view);
	gl_Position = ubo.proj * vec4( view * inPosition.xyz, 1.0);
    gl_Position.w = gl_Position.z;
}
