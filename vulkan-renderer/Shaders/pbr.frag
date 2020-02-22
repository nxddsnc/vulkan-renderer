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
layout(location = IN_UV0_LOCATION + 3) in vec3 inUv;
#endif

//output
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
    vec3 cameraPos;
} ubo;

layout(push_constant) uniform UniformPerDrawable
{
    layout(offset = 128) vec4 baseColor;
} uniformPerDrawable;

layout(set = 2, binding = 0) uniform samplerCube u_prefileteredCubemap;
layout(set = 2, binding = 1) uniform sampler2D   u_brdfLut;

#if TEXTURE_BASE_COLOR
layout(set = 3, binding = TEXUTRE_BASE_COLOR_LOCATION) uniform sampler2D baseColorTexture;
#endif
#if TEXTURE_NORMAL
layout(set = 3, binding = TEXTURE_NORMAL_LOCATION) uniform sampler2D normalTexture;
#endif


vec3 ApproximateSpecularIBL(vec3 color, float Roughness, vec3 N, vec3 V )
{
    float NoV = clamp(dot(N, V), 0, 1);
    vec3 R = 2 * dot(V, N) * N - V;
    
    const float MAX_REFLECTION_LOD = 9.0;
	float lod = Roughness * MAX_REFLECTION_LOD;
    
    vec4 prefilteredColor = textureLod(u_prefileteredCubemap, R, lod);
    vec2 brdf = texture(u_brdfLut, vec2(max(dot(N, V), 0.0), Roughness)).xy;
    return prefilteredColor.xyz * (color * brdf.x + brdf.y);
}

void main() {

#if IN_TANGENT && TEXTURE_NORMAL
    vec3 normal = texture(normalTexture, vec2(inUv.x, inUv.y)).xyz;
    vec3 worldNormal = inTBN * normal; 
#elif IN_UV0 && TEXTURE_NORMAL
    vec3 normal = texture(normalTexture, vec2(inUv.x, inUv.y)).xyz;
    vec3 pos_dx = dFdx(inPosition);
    vec3 pos_dy = dFdy(inPosition);
    vec3 tex_dx = dFdx(vec3(inUv.xy, 1.0));
    vec3 tex_dy = dFdy(vec3(inUv.xy, 1.0));
    vec3 t = (tex_dy.t * pos_dx - tex_dx.t * pos_dy) / (tex_dx.s * tex_dy.t - tex_dy.s * tex_dx.t);
    t = normalize(t - inNormal * dot(inNormal, t));
    vec3 b = normalize(cross(inNormal, t));
    mat3 tbn = mat3(t, b, inNormal);
    vec3 worldNormal = normalize(tbn * normal);
#else
    vec3 worldNormal = inNormal;
#endif

    vec3 V = ubo.cameraPos - inPosition;

    vec3 baseColor = vec3(1);

#if BASE_COLOR
    baseColor = uniformPerDrawable.baseColor.rgb;
#endif
#if TEXTURE_BASE_COLOR
    baseColor *= texture(baseColorTexture, vec2(inUv.x, -inUv.y)).xyz;
#endif

    outColor.rgb = ApproximateSpecularIBL(baseColor, 0.0, worldNormal, V);
    
    // outColor.rgb = worldNormal;
    // if (outColor.r > outColor.g && outColor.r > outColor.b)
    // {
    //     outColor = vec4(1, 0, 0, 1);
    // }
    // else if (outColor.g > outColor.r && outColor.g > outColor.b)
    // {
    //     outColor = vec4(0, 1, 0, 1);
    // }
    // else
    // {
    //     outColor = vec4(0, 0, 1, 1);
    // }
    outColor.a = 1.0;
}