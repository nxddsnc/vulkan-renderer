#version 450
#extension GL_ARB_separate_shader_objects : enable

// varyings
layout(location = 0) in vec3 inNormal;

#if IN_UV0
layout(location = IN_UV0_LOCATION) in vec2 inUv;
#endif

//output
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform LightUniforms {
    mat4 matrixR;
    mat4 matrixG;
    mat4 matrixB;
} lightUniforms;

#if TEXTURE_BASE_COLOR
layout(set = 2, binding = TEXUTRE_BASE_COLOR_LOCATION) uniform sampler2D baseColorTexture;
#endif

void main() {
    vec3 diffuse;
    diffuse.r = dot(vec4(inNormal, 1), lightUniforms.matrixR * vec4(inNormal, 1));   
    diffuse.g = dot(vec4(inNormal, 1), lightUniforms.matrixG * vec4(inNormal, 1));
    diffuse.b = dot(vec4(inNormal, 1), lightUniforms.matrixB * vec4(inNormal, 1));
    
    outColor = vec4(diffuse, 1.0);

    // outColor = vec4(inNormal, 1.0);
#if TEXTURE_BASE_COLOR
    outColor *= texture(baseColorTexture, vec2(inUv.x, -inUv.y));
#endif
}