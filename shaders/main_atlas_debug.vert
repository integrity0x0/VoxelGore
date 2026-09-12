#version 450

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
} oVert;

#include "UniformGameData.glsl"

struct UvRegion {
  vec4 uvRect;
  uint arrayLayer;
};

layout(binding = 0, set = 1) uniform BlockUvBuffer {
  UvRegion regions[512];
} uUv;

const vec2 CUBE_UVS[24] = vec2[](
    // 0: -Z
    vec2(1.0, 1.0), vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 0.0),

    // 1: +Z
    vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(1.0, 0.0), vec2(0.0, 0.0),

    // 2: -X
    vec2(1.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0),

    // 3: +X
    vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(1.0, 0.0), vec2(0.0, 1.0),

    // 4: -Y
    vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0),

    // 5: +Y
    vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(1.0, 0.0), vec2(0.0, 1.0)
);

void main() {
    const vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

    vec2 pos = positions[gl_VertexIndex];

    gl_Position = vec4(pos, 0.0, 1.0);
    oVert.uv = pos * 0.5 + 0.5;
    oVert.fog = 1.0f;
    oVert.light = vec3(1.0f);
    oVert.arrayLayer = 0.0f;
}