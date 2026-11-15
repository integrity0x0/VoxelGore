#version 460

precision highp float;
precision highp int;

layout(location = 0) in Vertex {
  mediump vec3 uv;
} iVert;

layout(binding = 0, set = 1) uniform samplerCube uSkyDay;
layout(binding = 1, set = 1) uniform samplerCube uSkySunset;
layout(binding = 2, set = 1) uniform samplerCube uSkyNight;

layout(push_constant) uniform Push {
  float timeOfDay; // 0..1
} pc;

layout(location = 0) out vec4 oFragColor;

void main() {
  float t = pc.timeOfDay;

  float sunrise = smoothstep(0.20, 0.30, t) * (1.0 - smoothstep(0.30, 0.45, t));
  float day     = smoothstep(0.30, 0.45, t) * (1.0 - smoothstep(0.55, 0.70, t));
  float sunset  = smoothstep(0.55, 0.70, t) * (1.0 - smoothstep(0.70, 0.80, t));
  float night   = 1.0 - (sunrise + day + sunset);

  vec3 colDay    = texture(uSkyDay,    iVert.uv).rgb;
  vec3 colSunset = texture(uSkySunset, iVert.uv).rgb;
  vec3 colNight  = texture(uSkyNight,  iVert.uv).rgb;
  vec3 color = colDay    * day +
               colSunset * (sunrise + sunset) +
               colNight  * night;

  oFragColor = vec4(color, 1.0);
}