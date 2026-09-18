#version 450

precision highp float;
precision highp int;

layout(location = 0) in VertexData {
  mediump vec2 uv;
  mediump vec4 color;
  mediump float fog;
  mediump vec3 fogColor;
} iVert;

layout(binding = 0, set = 1) uniform sampler2D uAlbedoTex;

layout(location = 0) out vec4 oFragColor;

void main() {
  oFragColor = texture(uAlbedoTex, iVert.uv) * iVert.color;
  oFragColor.rgb = mix(iVert.fogColor, oFragColor.rgb, iVert.fog);
}