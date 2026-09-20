#version 460

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

layout(location = 0) out vec2 oUV;
layout(location = 1) out vec4 oColor;

void main() {    
    gl_Position = vec4(aPos, 0.0f, 1.0f);
    
    oUV = aUV;
    oColor = aColor;
}

