#version 460

precision highp float;
precision highp int;

layout(location = 0) in VertexData {
  mediump vec2 uv;
  flat mediump float arrayLayer;
  mediump vec4 color;
} iVert;

layout(location = 0) out vec4 oFragColor;

layout(binding = 0, set = 1) uniform sampler2DArray uAtlas;

void main() {
  oFragColor = texture(uAtlas, vec3(iVert.uv, iVert.arrayLayer));
#ifdef CUTOUT_LAYER 
  if (oFragColor.a < 0.5f) discard;
#endif
  oFragColor *= iVert.color;
}