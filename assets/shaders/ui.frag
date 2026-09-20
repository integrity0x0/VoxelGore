#version 460

layout(location = 0) in vec2 iUV;
layout(location = 1) in vec4 iColor;

layout(binding = 0) uniform sampler2D uImage;

layout(location = 0) out vec4 oFragColor;

void main() {
    oFragColor = texture(uImage, iUV) * iColor;
}