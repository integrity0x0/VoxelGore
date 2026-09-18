#version 460

layout(location = 0) in vec2 iUV;

layout(location = 0) out vec4 oFragColor;

layout(binding = 0) uniform sampler2DArray uTextureAtlas;

void main() {
    oFragColor = texture(uTextureAtlas, vec3(iUV, 0.0f));
}