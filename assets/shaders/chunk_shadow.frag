#version 460

precision highp float;
precision highp int;

layout(binding = 1, set = 1) uniform sampler2DArray uTexture;

#ifdef CUTOUT_LAYER
layout(location = 0) in VertexData {
  mediump vec2 uv;
  mediump flat float arrayLayer;
} iVert;

void main() {
  if (texture(uTexture, vec3(iVert.uv, iVert.arrayLayer)).a < 0.5f) discard;
}
#else 
void main() {
}
#endif