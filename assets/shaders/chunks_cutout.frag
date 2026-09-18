#version 460

precision highp float;
precision highp int;

layout(location = 0) in VertexData {
  mediump vec2 uv;
  flat mediump float arrayLayer;
  mediump vec3 light;
  mediump float fog;
  mediump vec3 fogColor;
} iVert;

layout(location = 0) out vec4 oFragColor;

layout(binding = 1, set = 1) uniform sampler2DArray uTexture;

void main() {
    oFragColor = texture(uTexture, vec3(iVert.uv, iVert.arrayLayer));
    if(oFragColor.a < 0.5f) discard;
    oFragColor.rgb *= iVert.light;
    oFragColor.rgb = mix(iVert.fogColor, oFragColor.rgb, iVert.fog);
}