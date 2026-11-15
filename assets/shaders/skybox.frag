#version 460

precision highp float;
precision highp int;

layout(location = 0) in Vertex {
  mediump vec3 uv;
  flat mediump vec3 weights;
} iVert;

layout(binding = 0, set = 1) uniform samplerCube uSkyDay;
layout(binding = 0, set = 2) uniform samplerCube uSkySunset;
layout(binding = 0, set = 3) uniform samplerCube uSkyNight;

layout(location = 0) out vec4 oFragColor;

void main() {
  const vec3 w = iVert.weights;
  vec3 color = vec3(0.0);

  if (w.x > 0.001) color += texture(uSkyDay,    iVert.uv).rgb * w.x;
  if (w.y > 0.001) color += texture(uSkySunset, iVert.uv).rgb * w.y;
  if (w.z > 0.001) color += texture(uSkyNight,  iVert.uv).rgb * w.z;

  oFragColor = vec4(color, 1.0);
}