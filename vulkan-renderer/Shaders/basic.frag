#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

#if TEXTURE_BASE
layout(set = 1, binding = 0) uniform sampler2D baseColorTexture;
#endif

void main() {
    outColor = vec4(fragColor, 1.0);

#if TEXTURE_BASE
    outColor = texture(baseColorTexture, )
#endif
    // outColor = vec4(texture(texSampler, fragTexCoord).rgb * fragColor, 1.0);
    // outColor = vec4(fragTexCoord, 0.0, 1.0);
}