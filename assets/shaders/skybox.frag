#version 450

precision highp float;
precision highp int;

layout(location = 0) in Vertex {
  mediump vec3 uv;
} iVert;

layout(binding = 0, set = 1) uniform samplerCube uSkybox;

layout(location = 0) out vec4 oFragColor;

void main() {
  oFragColor = texture(uSkybox, iVert.uv);
}