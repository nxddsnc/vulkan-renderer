#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

#if IN_UV0
layout(location = IN_UV0_LOCATION) in vec2 inUv;
#endif

#if IN_TANGENT
layout(location = IN_TANGENT_LOCATION) in vec3 inTangent;
#endif

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.proj * ubo.view * vec4(inPosition, 1.0);
    // fragColor = inNormal;
#if IN_UV0
    fragColor = vec3(inUv, 0.0);
#else 
    fragColor = vec3(1.0, 0.0, 0.0);
#endif
}