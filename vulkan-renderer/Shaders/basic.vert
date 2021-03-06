#version 450

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

#if IN_NORMAL
layout(location = IN_NORMAL_LOCATION) in vec3 inNormal;
#endif

#if IN_UV0
layout(location = IN_UV0_LOCATION) in vec3 inUv;
#endif

#if IN_TANGENT
layout(location = IN_TANGENT_LOCATION) in vec3 inTangent;
#endif

#if IN_COLOR
layout(location = IN_COLOR_LOCATION) in vec3 inColor;
#endif
// varyings
#if IN_NORMAL && IN_TANGENT
// mat3 consumes 3 locations.
layout(location = 5) out mat3 outTBN;
#endif

layout(location = 0) out vec3 outPosition;

#if IN_NORMAL
layout(location = IN_NORMAL_LOCATION) out vec3 outNormal;
#endif

#if IN_UV0
layout(location = IN_UV0_LOCATION) out vec3 outUv;
#endif

#if IN_COLOR
layout(location = IN_COLOR_LOCATION) out vec3 outColor;
#endif

void main() {
    outPosition = (uniformPerDrawable.modelMatrix * vec4(inPosition, 1.0)).xyz;
    gl_Position = ubo.proj * ubo.view * vec4(outPosition, 1.0);

#if IN_NORMAL
    outNormal = (uniformPerDrawable.normalMatrix * vec4(inNormal, 1.0)).xyz;
#endif

#if IN_UV0
    outUv = inUv;
#endif

#if IN_TANGENT && IN_NORMAL
    vec3 tangent = (uniformPerDrawable.modelMatrix * vec4(inTangent, 0.0)).xyz;
    vec3 biTangent = normalize(cross(inNormal, tangent));
    outTBN = mat3(normalize(tangent), biTangent, inNormal);
#endif

#if IN_COLOR
    outColor = inColor;
#endif
}