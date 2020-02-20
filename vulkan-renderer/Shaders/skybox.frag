#version 450

layout (set = 1, binding = 0) uniform samplerCube samplerEnv;

layout (set = 2, binding = 0) uniform sampler2D samplerPreFilteredCubeMap;
// layout (set = 2, binding = 0) uniform samplerCube samplerPreFilteredCubeMap;

layout(location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outColor;

// layout (binding = 1) uniform UBOParams {
// 	vec4 lights[4];
// 	float exposure;
// 	float gamma;
// } uboParams;

// From http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 Uncharted2Tonemap(vec3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

void main() 
{
	vec3 color = texture(samplerEnv, inUVW).rgb;

	color = textureLod(samplerPreFilteredCubeMap, inUVW.xy, 0).rgb;
	// color = Uncharted2Tonemap(color);
	outColor = vec4(color, 1.0);

	// outColor = vec4(abs(inPosition.x + 0.5 - inUv.x), abs(inPosition.y + 0.5 - inUv.y), abs(inPosition.z + 0.5 - inUv.z), 1.0);

	// outColor = vec4(inUv.x, inUv.y, inUv.z, 1.0);
	// // Tone mapping
	// color = Uncharted2Tonemap(color * uboParams.exposure);
	// color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	// // Gamma correction
	// color = pow(color, vec3(1.0f / uboParams.gamma));
	
	// outColor = vec4(color, 1.0);
}