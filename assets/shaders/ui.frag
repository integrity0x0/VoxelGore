#version 460

precision highp float;
precision highp int;

layout(binding = 0) uniform sampler2D uImage;

layout(location = 0) in VertexData {
  mediump vec2 uv;
  mediump vec4 color;
  vec2 local;
  flat vec2 halfSize;
  flat float radius;
  flat float borderWidth;
} iVert;

layout(location = 0) out vec4 oFragColor;

float RoundRectSDF(vec2 p, vec2 halfSize, float radius) {
  vec2 q = abs(p) - (halfSize - radius);
  return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;
}

void main() {
  float d = RoundRectSDF(iVert.local, iVert.halfSize, iVert.radius);
  float coverage = clamp(0.5 - d, 0.0, 1.0);

  // borderWidth > 0 это квад рамки: закрашена только полоса шириной borderWidth у края.
  // -1 (или 0) это обычный квад, заливка целиком.
  if (iVert.borderWidth > 0.0) {
    coverage *= clamp(0.5 + d + iVert.borderWidth, 0.0, 1.0);
  }
  if (coverage <= 0.0) discard;  // центр рамки: нет ни текстуры, ни блендинга

  vec4 color = textureLod(uImage, iVert.uv, 0.0) * iVert.color;
  oFragColor = vec4(color.rgb, color.a * coverage);
}