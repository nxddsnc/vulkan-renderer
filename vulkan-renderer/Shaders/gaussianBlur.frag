#version 450

layout (binding = 0) uniform sampler2D colorTexture;

layout(push_constant) uniform UBO
{
	float blurScale;
	float blurStrength;
    vec2  blurDirection;
} m_uParams;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main() 
{
	
	float weight[6];
	weight[0] = 0.382925;
	weight[1] = 0.24173;
	weight[2] = 0.060598;
	weight[3] = 0.005977;
	weight[4] =	0.000229;
	weight[5] = 0.000003;

	vec2 tex_offset = 1.0 / textureSize(colorTexture, 0) * m_uParams.blurScale; // gets size of single texel
	vec3 result = texture(colorTexture, inUV).rgb * weight[0]; // current fragment's contribution
	for(int i = 1; i < 6; ++i)
	{
		result += texture(colorTexture, inUV + m_uParams.blurDirection * tex_offset * i).rgb * weight[i] * m_uParams.blurStrength;
		result += texture(colorTexture, inUV - m_uParams.blurDirection * tex_offset * i).rgb * weight[i] * m_uParams.blurStrength;
	}
	outColor = vec4(result, 1.0);

	// outColor = vec4(m_uParams.blurDirection, 0.0, 1.0);
	// outColor =  texture(colorTexture, inUV);
}