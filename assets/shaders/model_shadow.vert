#version 460

precision highp float;
precision highp int;

layout(location = 0) in vec3 aPos;
//layout(location = 1) in vec2 aUv;
//layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 instanceTransformCol0;
layout(location = 4) in vec3 instanceTransformCol1;
layout(location = 5) in vec3 instanceTransformCol2;
layout(location = 6) in vec3 instanceTransformCol3;
//layout(location = 7) in vec4 instanceColor;

#include "UniformGameData.glsl"

void main() {
  mat4x3 instanceTransform = mat4x3(instanceTransformCol0, instanceTransformCol1,
                                    instanceTransformCol2, instanceTransformCol3);
  vec4 worldPos = vec4(instanceTransform * vec4(aPos, 1.0), 1.0);

  gl_Position = uGameData.lightProjView * worldPos;
}