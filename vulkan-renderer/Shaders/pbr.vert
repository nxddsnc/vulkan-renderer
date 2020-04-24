#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
    vec3 cameraPos;
} ubo;

layout(set = 1, binding = 0) uniform UboJointMatrices {
    mat4 value[64];
} uboJointMatrices;

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

#if IN_JOINT
layout(location = IN_JOINT_LOCATION) in ivec4 inJoint;
#endif

#if IN_WEIGHT
layout(location = IN_WEIGHT_LOCATION) in vec4 inWeight;
#endif
// varyings
#if IN_TANGENT && IN_NORMAL
// mat3 consumes 3 location.
layout(location = 5) out mat3 outTBN;
#endif

layout(location = 0) out vec3 outPosition;

#if IN_NORMAL
layout(location = IN_NORMAL_LOCATION) out vec3 outNormal;
#endif

#if IN_UV0
layout(location = IN_UV0_LOCATION) out vec3 outUv;
#endif


void main() 
{
#if IN_JOINT
    vec4 temp = (  uboJointMatrices.value[inJoint.x] * vec4(inPosition, 1.0) * inWeight.x + 
                   uboJointMatrices.value[inJoint.y] * vec4(inPosition, 1.0) * inWeight.y + 
                   uboJointMatrices.value[inJoint.z] * vec4(inPosition, 1.0) * inWeight.z +
                   uboJointMatrices.value[inJoint.w] * vec4(inPosition, 1.0) * inWeight.w);

    // mat4 test = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    // outPosition = (uboJointMatrices.value[0] * vec4(inPosition, 1.0) * 1.0).xyz;

    temp = uniformPerDrawable.modelMatrix * temp;
    // outPosition = (uniformPerDrawable.modelMatrix * temp).xyz;
    gl_Position = ubo.proj * ubo.view * temp;
#else 
    outPosition = (uniformPerDrawable.modelMatrix * vec4(inPosition, 1.0)).xyz;
    gl_Position = ubo.proj * ubo.view * vec4(outPosition, 1.0);
#endif

#if IN_NORMAL
    outNormal = (uniformPerDrawable.normalMatrix * vec4(inNormal, 0.0)).xyz;
#endif

#if IN_UV0
    outUv = inUv;
#endif

#if IN_TANGENT
    vec3 tangent = (uniformPerDrawable.modelMatrix * vec4(inTangent, 0.0)).xyz;
    vec3 biTangent = normalize(cross(inNormal, tangent));
    outTBN = mat3(normalize(tangent), biTangent, inNormal);
#endif
}