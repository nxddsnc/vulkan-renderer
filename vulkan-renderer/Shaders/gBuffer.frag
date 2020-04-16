#version 450

// varyings
// mat3 consumes 3 locations.
#if IN_NORMAL && IN_TANGENT
layout(location = 5) in mat3 inTBN;
#endif

layout(location = 0) in vec3 inPosition;

#if IN_NORMAL
layout(location = IN_NORMAL_LOCATION) in vec3 inNormal;
#endif 

#if IN_UV0
layout(location = IN_UV0_LOCATION) in vec3 inUv;
#endif

//output
layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo; 

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
    vec3 cameraPos;
} ubo;

layout(push_constant) uniform UniformPerDrawable
{
    layout(offset = 128) vec4 baseColor;
    layout(offset = 144) vec2 metallicRoughness;
} uniformPerDrawable;

#if TEXTURE_BASE_COLOR
layout(set = 1, binding = TEXUTRE_BASE_COLOR_LOCATION) uniform sampler2D baseColorTexture;
#endif
#if TEXTURE_NORMAL
layout(set = 1, binding = TEXTURE_NORMAL_LOCATION) uniform sampler2D normalTexture;
#endif
#if TEXTURE_METALLIC_ROUGHNESS
layout(set = 1, binding = TEXTURE_METALLIC_ROUGHNESS_LOCATION) uniform sampler2D metallicRoughnessTexture;
#endif

void main() {

#if IN_TANGENT && TEXTURE_NORMAL
    vec3 normal = texture(normalTexture, vec2(inUv.x, inUv.y)).xyz * 2 - 1;
    vec3 worldNormal = inTBN * normal; 
#elif IN_UV0 && TEXTURE_NORMAL
    // http://www.thetenthplanet.de/archives/1180
    vec3 normal = texture(normalTexture, vec2(inUv.x, inUv.y)).xyz * 2 - 1;
    vec3 pos_dx = dFdx(inPosition);
    vec3 pos_dy = dFdy(inPosition);
    vec3 tex_dx = dFdx(vec3(inUv.xy, 1.0));
    vec3 tex_dy = dFdy(vec3(inUv.xy, 1.0));
    vec3 t = (tex_dy.t * pos_dx - tex_dx.t * pos_dy) / (tex_dx.s * tex_dy.t - tex_dy.s * tex_dx.t);
    t = normalize(t - inNormal * dot(inNormal, t));
    vec3 b = normalize(cross(inNormal, t));
    mat3 tbn = mat3(t, b, inNormal);
    vec3 worldNormal = normalize(tbn * normal);
#elif IN_NORMAL
    vec3 worldNormal = inNormal;
#else
    vec3 worldNormal = vec3(0);
#endif

    vec3 baseColor = vec3(1);

#if BASE_COLOR
    baseColor = uniformPerDrawable.baseColor.rgb;
#endif
#if TEXTURE_BASE_COLOR
    baseColor *= texture(baseColorTexture, vec2(inUv.x, inUv.y)).xyz;
#endif

    vec2 metallicRoughness = vec2(0, 1);
#if METALLIC_ROUGHNESS
    metallicRoughness = uniformPerDrawable.metallicRoughness;
#endif

#if TEXTURE_METALLIC_ROUGHNESS && IN_UV0
    metallicRoughness = texture(metallicRoughnessTexture, vec2(inUv.x, inUv.y)).bg;
#endif

    outPosition = vec4(inPosition, metallicRoughness.x);
    outNormal = vec4(worldNormal, metallicRoughness.y);
    outAlbedo = vec4(baseColor, 1.0);
}