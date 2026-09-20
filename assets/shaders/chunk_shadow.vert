#version 460

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

#ifdef CUTOUT_LAYER
layout(location = 0) out VertexData {
  mediump vec2 uv;
  mediump flat float arrayLayer;
} oVert;

#include "cube_uvs.glsl"
#endif

void main() {
  gl_Position = uGameData.lightProjView * vec4(aPos, 1.0);
#ifdef CUTOUT_LAYER
  uint uvIndex = aFaceIndex * 4u + aCornerIndex;
  vec2 localUV = CUBE_UVS[uvIndex];

  UvRegion region = uUv.regions[aSurfaceId];

  vec2 uvMin = region.uvRect.xy;
  vec2 uvMax = region.uvRect.zw;

  oVert.uv = mix(uvMin, uvMax, localUV);
  oVert.arrayLayer = float(region.arrayLayer);
#endif
}