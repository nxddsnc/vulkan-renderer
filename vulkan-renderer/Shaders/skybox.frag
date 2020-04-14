#version 450

layout (set = 1, binding = 0) uniform samplerCube samplerEnv;

// layout (set = 2, binding = 0) uniform samplerCube samplerPreFilteredCubeMap;
// layout (set = 2, binding = 0) uniform samplerCube samplerPreFilteredCubeMap;

layout(location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outColor;

// layout (binding = 1) uniform UBOParams {
// 	vec4 lights[4];
// 	float exposure;
// 	float gamma;
// } uboParams;

void main() 
{
	vec3 color = texture(samplerEnv, inUVW).rgb;

	// color = textureLod(samplerPreFilteredCubeMap, inUVW, 20).rgb;
	// color = Uncharted2Tonemap(color);
	outColor = vec4(color, 1.0);
	
	// outColor = vec4(inUVW, 1.0);

	// outColor = vec4(abs(inPosition.x + 0.5 - inUv.x), abs(inPosition.y + 0.5 - inUv.y), abs(inPosition.z + 0.5 - inUv.z), 1.0);

	// outColor = vec4(color, 1.0);
}