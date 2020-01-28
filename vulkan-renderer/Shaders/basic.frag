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
layout(location = IN_UV0_LOCATION + 3) in vec2 inUv;
#endif

//output
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform LightUniforms {
    mat4 matrixR;
    mat4 matrixG;
    mat4 matrixB;
} lightUniforms;

#if BASE_COLOR
layout(push_constant) uniform UniformPerDrawable
{
    layout(offset = 128) vec4 baseColor;
} uniformPerDrawable;
#endif

#if TEXTURE_BASE_COLOR
layout(set = 2, binding = TEXUTRE_BASE_COLOR_LOCATION) uniform sampler2D baseColorTexture;
#endif
#if TEXTURE_NORMAL
layout(set = 2, binding = TEXTURE_NORMAL_LOCATION) uniform sampler2D normalTexture;
#endif

void main() {

#if IN_TANGENT && TEXTURE_NORMAL
    vec3 normal = texture(normalTexture, vec2(inUv.x, -inUv.y)).xyz;
    vec3 worldNormal = inTBN * normal; 
#elif IN_UV0 && TEXTURE_NORMAL
    vec3 normal = texture(normalTexture, vec2(inUv.x, -inUv.y)).xyz;
    vec3 pos_dx = dFdx(inPosition);
    vec3 pos_dy = dFdy(inPosition);
    vec3 tex_dx = dFdx(vec3(inUv, 1.0));
    vec3 tex_dy = dFdy(vec3(inUv, 1.0));
    vec3 t = (tex_dy.t * pos_dx - tex_dx.t * pos_dy) / (tex_dx.s * tex_dy.t - tex_dy.s * tex_dx.t);
    t = normalize(t - inNormal * dot(inNormal, t));
    vec3 b = normalize(cross(inNormal, t));
    mat3 tbn = mat3(t, b, inNormal);
    vec3 worldNormal = normalize(tbn * normal);
#else
    vec3 worldNormal = inNormal;
#endif

    vec3 diffuse;
    diffuse.r = dot(vec4(worldNormal, 1), lightUniforms.matrixR * vec4(worldNormal, 1));   
    diffuse.g = dot(vec4(worldNormal, 1), lightUniforms.matrixG * vec4(worldNormal, 1));
    diffuse.b = dot(vec4(worldNormal, 1), lightUniforms.matrixB * vec4(worldNormal, 1));
    
    outColor = vec4(diffuse, 1.0);

#if BASE_COLOR
    outColor = uniformPerDrawable.baseColor * outColor;
#endif

    // outColor = vec4(inNormal, 1.0);
#if TEXTURE_BASE_COLOR
    outColor *= texture(baseColorTexture, vec2(inUv.x, -inUv.y));
#endif

// #if TEXTURE_NORMAL
//     outColor = texture(normalTexture, vec2(inUv.x, -inUv.y));
// #endif

}