#version 450

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2D colorTexture;

layout(push_constant) uniform UBO
{
	float threshold;
} m_uParams;

void main() 
{
	vec4 color = texture(colorTexture, inUV);
	float luminance = color.r * 0.3 + color.g * 0.59 + color.b * 0.11;
	color = color * step(m_uParams.threshold, luminance);
	outColor = color;
}