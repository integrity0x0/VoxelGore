#version 460

precision highp float;
precision highp int;

layout(location = 0) in highp vec3 aPos;
layout(location = 1) in mediump vec4 aLight;
layout(location = 2) in uint aFaceIndex;
layout(location = 3) in uint aCornerIndex;
layout(location = 4) in uint aSurfaceId;

layout(location = 0) out VertexData {
  mediump vec2 uv;
  flat mediump float arrayLayer;
  mediump vec3 light;
  mediump float fog;
  mediump vec3 fogColor;

#ifdef SHADOWS_ENABLED
  flat uint shadowEnabled;
  highp vec4 shadowCoord;
#endif
} oVert;

#include "UniformGameData.glsl"

struct UvRegion {
  vec4 uvRect;
  uint arrayLayer;
};

layout(binding = 0, set = 1) uniform BlockUvBuffer {
  UvRegion regions[512];
} uUv;

#include "cube_uvs.glsl"

const vec3 kFaceNormals[6] = vec3[](
  vec3( 0,  0, -1),
  vec3( 0,  0,  1),
  vec3(-1,  0,  0),
  vec3( 1,  0,  0),
  vec3( 0, -1,  0),
  vec3( 0,  1, 0)
);

void main() {
  uint uvIndex = aFaceIndex * 4u + aCornerIndex;
  vec2 localUV = CUBE_UVS[uvIndex];

  UvRegion region = uUv.regions[aSurfaceId];

  vec2 uvMin = region.uvRect.xy;
  vec2 uvMax = region.uvRect.zw;

  oVert.uv = mix(uvMin, uvMax, localUV);
  oVert.arrayLayer = float(region.arrayLayer);

  vec3 faceNormal = kFaceNormals[aFaceIndex];

  vec3 sunLight = uGameData.ambientColor * aLight.a;
  oVert.light = clamp(aLight.rgb + sunLight, 0.0, 1.0);

  const float GAMMA = 0.6;
  oVert.light = pow(oVert.light, vec3(1.0 / GAMMA));

  vec4 worldPos = vec4(aPos, 1.0);

  float dist = distance(worldPos.xyz, uGameData.cameraPos);

  float fogFactor = exp2(
      -uGameData.fogDensity *
       uGameData.fogDensity *
       dist * dist *
       1.442695
  );

  oVert.fog = clamp(fogFactor, 0.0, 1.0);
  oVert.fogColor = uGameData.ambientColor;

#ifdef SHADOWS_ENABLED
  float lightFacing = dot(faceNormal, -uGameData.lightDir);
  oVert.shadowEnabled = uint(lightFacing > 0.0);

  if (oVert.shadowEnabled != 0) {
    vec3 offsetWorldPos = worldPos.xyz + faceNormal * 0.4;

    vec4 lightSpacePos =
        uGameData.lightProjView * vec4(offsetWorldPos, 1.0);

    oVert.shadowCoord = lightSpacePos / lightSpacePos.w;
    oVert.shadowCoord.xy = oVert.shadowCoord.xy * 0.5 + 0.5;
  }
#endif

  gl_Position = uGameData.projView * worldPos;
}