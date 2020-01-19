#version 450
#extension GL_ARB_separate_shader_objects : enable

// varyings
#if IN_UV0
layout(location = IN_UV0_LOCATION) out vec2 inUv;
#endif

//output
layout(location = 0) out vec4 outColor;

#if TEXTURE_BASE_COLOR
layout(set = 1, binding = 0) uniform sampler2D baseColorTexture;
#endif

void main() {
    outColor = vec4(1.0);
    outColor = vec4(inUv, 0.0, 1.0);
#if TEXTURE_BASE_COLOR
    outColor = texture(baseColorTexture, inUv);
#endif
}