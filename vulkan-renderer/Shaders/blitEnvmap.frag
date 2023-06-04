#version 450

layout (location = 0) in vec3 inPos;
layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2D samplerEnv;


const float PI = 3.1415926536;

vec2 dirToUV(vec3 dir)
{
	vec3 normalizedDir = normalize(dir);
	return vec2(
		0.5f + 0.5f * atan(normalizedDir.y, normalizedDir.x) / PI,
		acos(normalizedDir.z) / PI);
}

void main()
{		
	vec2 UV = dirToUV(normalize(inPos));
	outColor = textureLod(samplerEnv, UV, 0.0);
}
