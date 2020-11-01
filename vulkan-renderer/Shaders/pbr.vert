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

#if INSTANCE_ENABLED
layout(location = IN_MATRIX_0) in vec4 matrix0;
layout(location = IN_MATRIX_1) in vec4 matrix1;
layout(location = IN_MATRIX_2) in vec4 matrix2;
#else
layout(push_constant) uniform UniformPerDrawable
{
    mat4 modelMatrix;
    mat4 normalMatrix;
} uniformPerDrawable;
#endif 

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
    mat4 jointMatrix = (  uboJointMatrices.value[inJoint.x] * inWeight.x + 
                   uboJointMatrices.value[inJoint.y] * inWeight.y + 
                   uboJointMatrices.value[inJoint.z] * inWeight.z +
                   uboJointMatrices.value[inJoint.w] * inWeight.w);

    // mat4 test = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    // outPosition = (uboJointMatrices.value[0] * vec4(inPosition, 1.0) * 1.0).xyz;

    vec4 temp = uniformPerDrawable.modelMatrix * jointMatrix * vec4(inPosition, 1.0);
    // outPosition = (uniformPerDrawable.modelMatrix * temp).xyz;
    gl_Position = ubo.proj * ubo.view * temp;


#if IN_NORMAL
    mat4 normalMatrix = inverse(jointMatrix);
    outNormal = (uniformPerDrawable.normalMatrix * normalMatrix * vec4(inNormal, 0.0)).xyz;
#endif

#else 

#if INSTANCE_ENABLED
    mat4 mat = mat4(matrix0[0], matrix1[0], matrix2[0], 0.0,
                    matrix0[1], matrix1[1], matrix2[1], 0.0,
                    matrix0[2], matrix1[2], matrix2[2], 0.0,
                    matrix0[3], matrix1[3], matrix2[3], 1.0);

    outPosition = (mat * vec4(inPosition, 1.0)).xyz;
    gl_Position = ubo.proj * ubo.view * vec4(outPosition, 1.0);

    #if IN_NORMAL
    mat3 normal_mat = mat3(matrix0.xyz, matrix1.xyz, matrix2.xyz);
    normal_mat = inverse(normal_mat);
    outNormal = (normal_mat * inNormal).xyz;
    #endif

#else 
    outPosition = (uniformPerDrawable.modelMatrix * vec4(inPosition, 1.0)).xyz;
    gl_Position = ubo.proj * ubo.view * vec4(outPosition, 1.0);

#if IN_NORMAL
    outNormal = (uniformPerDrawable.normalMatrix * vec4(inNormal, 0.0)).xyz;
#endif

#endif

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