#version 450

precision highp float;
precision highp int;

layout(location = 0) in vec3 aPos;

layout(binding = 0, set = 0) uniform UniformGameData {
  mat4 projView;
  mat4 proj;
  mat4 view;
  vec3 cameraPos;
  vec3 cameraDir;
} uGameData;

layout(location = 0) out Vertex {
  mediump vec3 uv;
} oVert;

void main() {
  mat4 view = mat4(mat3(uGameData.view));
  gl_Position = uGameData.proj * view * vec4(aPos, 1.0f);
  gl_Position.z = gl_Position.w;
  oVert.uv = aPos;
}