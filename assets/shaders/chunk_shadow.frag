#version 460

precision highp float;
precision highp int;

layout(binding = 1, set = 1) uniform sampler2DArray uTexture;

#ifdef CUTOUT_LAYER
layout(location = 0) out VertexData {
  mediump vec2 uv;
  mediump flat float layer;
} oVert;

void main() {
  if (texture(uTexture, vec3(uv, layer)).a < 0.5f) discard;
}
#else 
void main() {
}
#endif