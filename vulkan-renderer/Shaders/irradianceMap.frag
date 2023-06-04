#version 450

layout (location = 0) in vec3 inPos;
layout (location = 0) out vec4 outColor;

#if IS_CUBEMAP
layout (binding = 0) uniform samplerCube samplerEnv;
#else
layout (binding = 0) uniform sampler2D samplerEnv;
#endif

const float PI = 3.1415926535897932384626433832795;

vec2 dirToUV(vec3 dir)
{
	vec3 normalizedDir = normalize(dir);
	return vec2(
		0.5f + 0.5f * atan(normalizedDir.y, normalizedDir.x) / PI,
		acos(normalizedDir.z) / PI);
}

void main()
{		
	vec3 N = normalize(inPos);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up = cross(N, right);

	const float TWO_PI = PI * 2.0;
	const float HALF_PI = PI * 0.5;
	const float deltaPhi = TWO_PI / 360.0;
	const float deltaTheta = HALF_PI / 180.0;

	vec3 color = vec3(0.0);
	uint sampleCount = 0u;
	for (float phi = 0.0; phi < TWO_PI; phi += deltaPhi) {
		for (float theta = 0.0; theta < HALF_PI; theta += deltaTheta) {
			vec3 tempVec = cos(phi) * right + sin(phi) * up;
			vec3 sampleVector = cos(theta) * N + sin(theta) * tempVec;
#if IS_CUBEMAP
			color += texture(samplerEnv, sampleVector).rgb * cos(theta) * sin(theta);
#else
			color += texture(samplerEnv, dirToUV(sampleVector)).rgb * cos(theta) * sin(theta);
#endif
			sampleCount++;
		}
	}

	// 2 * PI * color * cos(theta) * sin(theta) * d(theta)
	outColor = vec4(PI * color / float(sampleCount), 1.0);
}
