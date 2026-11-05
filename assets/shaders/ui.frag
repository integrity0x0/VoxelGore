#version 460

precision highp float;
precision highp int;

layout(binding = 0) uniform sampler2D uImage;

layout(location = 0) in VertexData {
  vec2 uv;
  mediump vec4 color;
  vec2 local;
  flat vec2 halfSize;
  flat float radius;
  flat float borderWidth;
  flat uint type;
} iVert;

layout(location = 0) out vec4 oFragColor;

float RoundRectSDF(vec2 p, vec2 halfSize, float radius) {
  vec2 q = abs(p) - (halfSize - radius);
  return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;
}

void main() {
  vec4 textureColor = textureLod(uImage, iVert.uv, 0.0);

  float coverage;
  vec4 color;

  if (iVert.type == 0u) {
    float d = RoundRectSDF(
      iVert.local,
      iVert.halfSize,
      iVert.radius
    );

    coverage = clamp(0.5 - d, 0.0, 1.0);

    if (iVert.borderWidth > 0.0) {
      coverage *= clamp(
        0.5 + d + iVert.borderWidth,
        0.0,
        1.0
      );
    }

    color = textureColor * iVert.color;
  } else {
    coverage = textureColor.a;

    color = iVert.color;
  }

  if (coverage <= 0.0) discard;

  oFragColor = vec4(
    color.rgb,
    color.a * coverage
  );
}