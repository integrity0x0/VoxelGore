#version 460

layout(location = 0) in vec3 iPos;
layout(location = 1) in vec2 iUV;

layout(push_constant) uniform MVP {
    mat4 mvp;
} ubo;

layout(location = 0) out vec2 oUV;

void main() {
    oUV = iUV;
    gl_Position = ubo.mvp * vec4(iPos, 1.0);
}