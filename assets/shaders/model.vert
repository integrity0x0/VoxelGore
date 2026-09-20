#version 460

precision highp float;
precision highp int;

layout(location = 0) in highp vec3 aPos;
layout(location = 1) in mediump vec2 aUv;
layout(location = 2) in highp vec3 aNormal;
layout(location = 3) in vec3 instanceTransformCol0;
layout(location = 4) in vec3 instanceTransformCol1;
layout(location = 5) in vec3 instanceTransformCol2;
layout(location = 6) in vec3 instanceTransformCol3;
layout(location = 7) in vec4 instanceLight;

#include "UniformGameData.glsl"

layout(location = 0) out VertexData {
  mediump vec2 uv;
  mediump vec4 color;
  mediump vec3 light;
  mediump float fog;
  mediump vec3 fogColor;

#ifdef SHADOWS_ENABLED
  flat uint shadowEnabled;
  highp vec4 shadowCoord;
#endif
} oVert;

void main() {
  mat4x3 instanceTransform = mat4x3(
      instanceTransformCol0,
      instanceTransformCol1,
      instanceTransformCol2,
      instanceTransformCol3);

  vec4 worldPos = vec4(instanceTransform * vec4(aPos, 1.0), 1.0);

  gl_Position = uGameData.projView * worldPos;

  oVert.uv = aUv;
  oVert.color = instanceLight;

  vec3 sunLight = uGameData.ambientColor * instanceLight.a;

  oVert.light = clamp(instanceLight.rgb + sunLight, 0.0, 1.0);

  const float GAMMA = 0.6;
  oVert.light = pow(oVert.light, vec3(1.0 / GAMMA));

  float dist = distance(worldPos.xyz, uGameData.cameraPos);

  float fogFactor = exp2(
      -uGameData.fogDensity * uGameData.fogDensity * dist * dist * 1.442695);

  oVert.fog = clamp(fogFactor, 0.0, 1.0);
  oVert.fogColor = uGameData.ambientColor;

#ifdef SHADOWS_ENABLED
  float lightFacing = dot(aNormal, -uGameData.lightDir);

  oVert.shadowEnabled = uint(lightFacing > 0.0);

  if (oVert.shadowEnabled != 0) {
    vec3 offsetWorldPos = worldPos.xyz + aNormal * 0.4;

    vec4 lightSpacePos = uGameData.lightProjView * vec4(offsetWorldPos, 1.0);

    oVert.shadowCoord = lightSpacePos / lightSpacePos.w;

    oVert.shadowCoord.xy = oVert.shadowCoord.xy * 0.5 + 0.5;
  }
#endif
}