#version 460

precision highp float;
precision highp int;

layout(location = 0) in VertexData {
  mediump vec2 uv;
  flat mediump float arrayLayer;
  mediump vec3 light;
  mediump float fog;
} iVert;

layout(location = 0) out vec4 oFragColor;

layout(binding = 1, set = 1) uniform sampler2DArray uTexture;

const vec3 FOG_COLOR = vec3(0.53, 0.81, 0.92);

void main() {
    oFragColor = texture(uTexture, vec3(iVert.uv, iVert.arrayLayer));
    oFragColor.rgb *= iVert.light;
    oFragColor.rgb = mix(FOG_COLOR, oFragColor.rgb, iVert.fog);
}