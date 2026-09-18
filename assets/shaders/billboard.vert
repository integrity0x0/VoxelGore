#version 460

precision highp float;
precision highp int;

layout(location = 0) in vec3 aPos;
layout(location = 1) in float aRotation;
layout(location = 2) in vec2 aSize;
layout(location = 3) in mediump vec4 aUvMinMax;
layout(location = 4) in mediump float aLayer;
layout(location = 5) in mediump vec4 aColor;

const vec2 kCorners[4] = vec2[4](
    vec2(-0.5, -0.5), vec2(0.5, -0.5),
    vec2(-0.5,  0.5), vec2(0.5,  0.5));

layout(binding = 0, set = 0) uniform UniformGameData {
  mat4 projView;
  mat4 proj;
  mat4 view;
  vec3 cameraPos;
  vec3 cameraDir;
} uGameData;


layout(location = 0) out VertexData {
  mediump vec2 uv;
  flat mediump float arrayLayer;
  mediump vec4 color; 
} oVert;

void main() {
    vec2 corner = kCorners[gl_VertexIndex];

    vec3 cameraRight = vec3(uGameData.view[0][0], uGameData.view[1][0], uGameData.view[2][0]);
    vec3 cameraUp    = vec3(uGameData.view[0][1], uGameData.view[1][1], uGameData.view[2][1]);

    vec3 worldPos = aPos
        + cameraRight * (corner.x * aSize.x)
        + cameraUp    * (corner.y * aSize.y);

    gl_Position = uGameData.proj * uGameData.view * vec4(worldPos, 1.0);

    // Интерполяция UV с инверсией V для Vulkan
    float u = mix(aUvMinMax.x, aUvMinMax.z, corner.x + 0.5);
    float v = mix(aUvMinMax.y, aUvMinMax.w, 1.0 - (corner.y + 0.5));
    oVert.uv = vec2(u, v);

    oVert.arrayLayer = aLayer;
    oVert.color = aColor;
}