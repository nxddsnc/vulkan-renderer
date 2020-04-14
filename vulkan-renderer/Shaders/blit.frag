#version 450

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D blurredTexture;
layout (set = 1, binding = 0) uniform sampler2D colorTexture;
// From http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main() 
{
    outColor = texture(blurredTexture, inUV);
    outColor += texture(colorTexture, inUV);
    // Tone mapping
    float exposure = 2.0;
	outColor.rgb = Uncharted2Tonemap(outColor.rgb * exposure);
	outColor.rgb = outColor.rgb * (1.0 / Uncharted2Tonemap(vec3(11.2f)));	
	// Gamma correction
	outColor.rgb = pow(outColor.rgb, vec3(1.0f / 2.2));
}