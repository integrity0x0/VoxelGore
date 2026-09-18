#version 450

precision highp float;
precision highp int;

layout(location = 0) in highp vec3 aPos;
layout(location = 1) in mediump vec4 aLight;
layout(location = 2) in uint aFaceIndex;
layout(location = 3) in uint aCornerIndex;
layout(location = 4) in uint aSurfaceId;

#include "UniformGameData.glsl"

struct UvRegion {
  vec4 uvRect;
  uint arrayLayer;
};

layout(binding = 0, set = 1) uniform BlockUvBuffer {
  UvRegion regions[512];
} uUv;

void main() {
  gl_Position = uGameData.projView * vec4(aPos, 1.0);
}