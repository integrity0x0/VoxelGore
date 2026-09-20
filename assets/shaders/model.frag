#version 460

precision highp float;
precision highp int;

layout(location = 0) in VertexData {
  mediump vec2 uv;
  mediump vec4 color;
  mediump vec3 light;
  mediump float fog;
  mediump vec3 fogColor;

#ifdef SHADOWS_ENABLED
  flat uint shadowEnabled;
  highp vec4 shadowCoord;
#endif
} iVert;

layout(binding = 0, set = 1) uniform sampler2D uAlbedoTex;

#ifdef SHADOWS_ENABLED

layout(binding = 0, set = 2) uniform sampler2DShadow uShadowMap;

#include "shadow.glsl"

#endif

layout(location = 0) out vec4 oFragColor;

void main() {
  oFragColor = texture(uAlbedoTex, iVert.uv) * iVert.color;

#ifdef SHADOWS_ENABLED
  if (iVert.shadowEnabled != 0) {
    const float shadow = CalcShadow(uShadowMap, iVert.shadowCoord);

    oFragColor.rgb *= mix(kShadowColor, vec3(1.0), shadow);
  } else {
    oFragColor.rgb *= kShadowColor;
  }
#endif

  oFragColor.rgb *= iVert.light;

  oFragColor.rgb = mix(iVert.fogColor, oFragColor.rgb, iVert.fog);
}