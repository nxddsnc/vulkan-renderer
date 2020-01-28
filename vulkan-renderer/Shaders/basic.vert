#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
} ubo;

layout(push_constant) uniform UniformPerDrawable
{
    mat4 modelMatrix;
    mat4 normalMatrix;
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
layout(location = 0) out mat3 outTBN;
layout(location = 3) out vec3 outPosition;
layout(location = 4) out vec3 outNormal;

#if IN_UV0
layout(location = IN_UV0_LOCATION + 3) out vec2 outUv;
#endif

void main() {
    outPosition = (uniformPerDrawable.modelMatrix * vec4(inPosition, 1.0)).xyz;
    gl_Position = ubo.proj * ubo.view * vec4(outPosition, 1.0);
    outNormal = (uniformPerDrawable.normalMatrix * vec4(inNormal, 1.0)).xyz;
#if IN_UV0
    outUv = inUv;
#endif

#if IN_TANGENT
    vec3 tangent = (uniformPerDrawable.modelMatrix * vec4(inTangent, 0.0)).xyz;
    vec3 biTangent = normalize(cross(inNormal, tangent));
    outTBN = mat3(normalize(tangent), biTangent, inNormal);
#endif
}