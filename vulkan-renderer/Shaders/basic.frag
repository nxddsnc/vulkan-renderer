#version 450
#extension GL_ARB_separate_shader_objects : enable

// varyings
#if IN_UV0
layout(location = IN_UV0_LOCATION) in vec2 inUv;
#endif

//output
layout(location = 0) out vec4 outColor;

#if TEXTURE_BASE_COLOR
layout(set = 1, binding = TEXUTRE_BASE_COLOR_LOCATION) uniform sampler2D baseColorTexture;
#endif

void main() {
    // outColor = vec4(1.0);
    // outColor = vec4(-inUv.y, inUv.y, 0.0, 1.0);
#if TEXTURE_BASE_COLOR
    outColor = texture(baseColorTexture, vec2(inUv.x, -inUv.y));
#endif
}