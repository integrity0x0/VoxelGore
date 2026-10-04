#version 460

precision highp float;
precision highp int;

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aSize;
layout(location = 2) in vec4 aUvRect;
layout(location = 3) in vec4 aColor;
layout(location = 4) in float aRadius;

layout(push_constant) uniform PushData {
  vec2 ndcScale;
} pc;

layout(location = 0) out VertexData {
  mediump vec2 uv;
  mediump vec4 color;
  vec2 local;
  flat vec2 halfSize;
  flat float radius;
} oVert;

const vec2 kCorners[6] = vec2[](vec2(0, 0), vec2(0, 1), vec2(1, 1),
                               vec2(0, 0), vec2(1, 1), vec2(1, 0));

void main() {
  vec2 c = kCorners[gl_VertexIndex];
  gl_Position = vec4((aPos + c * aSize) * pc.ndcScale - 1.0, 0.0, 1.0);
  oVert.local = (c - 0.5) * aSize;
  oVert.uv = mix(aUvRect.xy, aUvRect.zw, c);
  oVert.color = aColor;
  oVert.halfSize = aSize * 0.5;
  oVert.radius = aRadius;
}