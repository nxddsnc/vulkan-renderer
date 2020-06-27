#version 450

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
    vec3 cameraPos;
} ubo;

layout(set = 5, binding = 0) uniform ShadowCamera {
    mat4 proj;
    mat4 view;
    vec3 cameraPos;
} shadowCameraUbo;

layout(set = 1, binding = 0) uniform LightUniforms {
    mat4 matrixR;
    mat4 matrixG;
    mat4 matrixB;
} lightUniforms;

layout(set = 2, binding = 0) uniform samplerCube u_prefileteredCubemap;
layout(set = 2, binding = 1) uniform samplerCube u_IrradianceMap;
layout(set = 2, binding = 2) uniform sampler2D   u_brdfLut;

layout(set = 3, binding = 0) uniform sampler2D   u_positionTexture;
layout(set = 3, binding = 1) uniform sampler2D   u_normalTexture;
layout(set = 3, binding = 2) uniform sampler2D   u_albedoTexture;

layout(set = 4, binding = 0) uniform sampler2DShadow u_shadowMap;

vec3 F_Schlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 ApproximateSpecularIBL(vec3 color, float Roughness, vec3 N, vec3 V)
{
    // vec3 R = (2 * dot(V, N) * N - V).xyz;
    vec3 R = -reflect(V, N).xzy;
    
    const float MAX_REFLECTION_LOD = 9.0;
	float lod = Roughness * MAX_REFLECTION_LOD;
    
    vec4 prefilteredColor = textureLod(u_prefileteredCubemap, R, lod);
    vec2 brdf = texture(u_brdfLut, vec2(clamp(dot(N, V), 0.0, 0.99), Roughness)).xy;
    // multiplied by length(N) to prevent background flicking.
    return length(N) * (prefilteredColor.xyz * (color * brdf.x + brdf.y));
}

float shadow(vec3 pos, vec2 texelScale)
{
    // float depth, sum = 0.0, x, y;
    // for (y = -1.5; y <= 1.5; y += 1.0)  
    // {
	//     for (x = -1.5; x <= 1.5; x += 1.0) 
	//     {
    //         depth = texture(u_shadowMap, pos.xyz + vec2(x, y) * texelScale);
    //         sum += 1.0 - 0.3 * step(depth, pos.z);
    //     }
    // }
    // return sum / 16;
    // float shadow = texture(u_shadowMap, pos + vec3(-texelScale.x, 0, 0));
    float shadow = texture(u_shadowMap, pos);
    shadow += texture(u_shadowMap, pos + vec3(-texelScale.x, 0, 0));
    shadow += texture(u_shadowMap, pos + vec3(texelScale.x, 0, 0));
    shadow += texture(u_shadowMap, pos + vec3(0, -texelScale.y, 0));
    shadow += texture(u_shadowMap, pos + vec3(0, texelScale.y, 0));
    return 1 - 0.3 * shadow / 5;
}

void main() 
{
    vec4 inPosition = texture(u_positionTexture, inUV);
    vec4 worldNormal = texture(u_normalTexture, inUV);
    vec3 baseColor = texture(u_albedoTexture, inUV).xyz;

    vec3 V = normalize(ubo.cameraPos - inPosition.xyz);

    vec2 metallicRoughness = vec2(inPosition.w, worldNormal.w);

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, baseColor.rgb, metallicRoughness.x);

    vec3 F = F_SchlickR(max(dot(worldNormal.xyz, V), 0.0), F0, metallicRoughness.y);

    outColor.rgb = ApproximateSpecularIBL(F, metallicRoughness.y, worldNormal.xyz, V);
    
    vec3 irradiance;

#if USE_IRRADIANCE_MAP
    irradiance = texture(u_IrradianceMap, worldNormal.xzy).rgb;
#else
    irradiance.r = dot(vec4(worldNormal.xzy, 1), lightUniforms.matrixR * vec4(worldNormal.xzy, 1));   
    irradiance.g = dot(vec4(worldNormal.xzy, 1), lightUniforms.matrixG * vec4(worldNormal.xzy, 1));
    irradiance.b = dot(vec4(worldNormal.xzy, 1), lightUniforms.matrixB * vec4(worldNormal.xzy, 1));
#endif

    vec3 diffuse = baseColor * irradiance * (1 - F) * (1 - metallicRoughness.x);

    outColor.rgb += diffuse;

    vec4 pos = shadowCameraUbo.proj * shadowCameraUbo.view * vec4(inPosition.xyz, 1.0);
    pos /= pos.w;
    pos.xy = (1.0 + pos.xy) * 0.5;

    vec2 texelScale = 1.0 / textureSize(u_shadowMap, 0);
    outColor *= shadow(pos.xyz, texelScale);

    outColor.a = 1.0;

}