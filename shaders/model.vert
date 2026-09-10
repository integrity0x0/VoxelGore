#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 instanceTransformCol0;
layout(location = 4) in vec3 instanceTransformCol1;
layout(location = 5) in vec3 instanceTransformCol2;
layout(location = 6) in vec3 instanceTransformCol3;
layout(location = 7) in vec4 instanceColor;

layout(binding = 0, set = 0) uniform UniformGameData {
  mat4 projView;
  mat4 proj;
  mat4 view;
  vec3 cameraPos;
  vec3 cameraDir;
} uGameData;

layout(location = 0) out VertexData {
  mediump vec2 uv;
  mediump vec4 color;
} oVert;

void main() {
  mat4x3 instanceTransform = mat4x3(instanceTransformCol0, instanceTransformCol1,
                                     instanceTransformCol2, instanceTransformCol3);
  vec4 worldPos = vec4(instanceTransform * vec4(aPos, 1.0), 1.0);

  gl_Position = uGameData.projView * worldPos;
  oVert.uv = aUv;
  oVert.color = instanceColor;
}