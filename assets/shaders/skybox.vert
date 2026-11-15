#version 460

precision highp float;
precision highp int;

layout(location = 0) in vec3 aPos;

#include "UniformGameData.glsl"

layout(location = 0) out Vertex {
  mediump vec3 uv;
  flat mediump vec3 weights;  // x = day, y = sunset, z = night
} oVert;

void main() {
  mat4 view = mat4(mat3(uGameData.view));
  gl_Position = uGameData.proj * view * vec4(aPos, 1.0);
  gl_Position.z = gl_Position.w * (1.0 - 1e-5);
  oVert.uv = aPos;
  oVert.weights = uGameData.skyWeights.xyz;
}