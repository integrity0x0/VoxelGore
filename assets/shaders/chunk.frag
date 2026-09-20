#version 460

precision highp float;
precision highp int;

layout(location = 0) in VertexData {
  mediump vec2 uv;
  flat mediump float arrayLayer;
  mediump vec3 light;
  mediump float fog;
  mediump vec3 fogColor;

#ifdef SHADOWS_ENABLED
  flat uint shadowEnabled;
  highp vec4 shadowCoord;
#endif
} iVert;

layout(location = 0) out vec4 oFragColor;

layout(binding = 1, set = 1) uniform sampler2DArray uTexture;

#ifdef SHADOWS_ENABLED

layout(binding = 0, set = 2) uniform sampler2DShadow uShadowMap;

#include "shadow.glsl"

#endif

void main() {
  oFragColor = texture(
      uTexture,
      vec3(iVert.uv, iVert.arrayLayer));

#ifdef CUTOUT_LAYER
  if (oFragColor.a < 0.5) {
    discard;
  }
#endif

#ifdef SHADOWS_ENABLED
  if (iVert.shadowEnabled != 0) {
    const float shadow = CalcShadow(
        uShadowMap,
        iVert.shadowCoord);

    oFragColor.rgb *= mix(
        kShadowColor,
        vec3(1.0),
        shadow);
  } else {
    oFragColor.rgb *= kShadowColor;
  }
#endif

  oFragColor.rgb *= iVert.light;

  oFragColor.rgb = mix(
      iVert.fogColor,
      oFragColor.rgb,
      iVert.fog);
}