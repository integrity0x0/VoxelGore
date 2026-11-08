

// ========== all_sources.cpp ==========



// ========== billboard.frag ==========

#version 460

precision highp float;
precision highp int;

layout(location = 0) in VertexData {
  mediump vec2 uv;
  flat mediump float arrayLayer;
  mediump vec4 color;
} iVert;

layout(location = 0) out vec4 oFragColor;

layout(binding = 0, set = 1) uniform sampler2DArray uAtlas;

void main() {
  oFragColor = texture(uAtlas, vec3(iVert.uv, iVert.arrayLayer));
#ifdef CUTOUT_LAYER 
  if (oFragColor.a < 0.5f) discard;
#endif
  oFragColor *= iVert.color;
}


// ========== billboard.vert ==========

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
    
    float u = mix(aUvMinMax.x, aUvMinMax.z, corner.x + 0.5);
    float v = mix(aUvMinMax.y, aUvMinMax.w, 1.0 - (corner.y + 0.5));
    oVert.uv = vec2(u, v);

    oVert.arrayLayer = aLayer;
    oVert.color = aColor;
}


// ========== blockPreview.frag ==========

#version 460

layout(location = 0) in vec2 iUV;

layout(location = 0) out vec4 oFragColor;

layout(binding = 0) uniform sampler2DArray uTextureAtlas;

void main() {
    oFragColor = texture(uTextureAtlas, vec3(iUV, 0.0f));
}


// ========== blockPreview.vert ==========

#version 460

layout(location = 0) in vec3 iPos;
layout(location = 1) in vec2 iUV;

layout(push_constant) uniform MVP {
    mat4 mvp;
} ubo;

layout(location = 0) out vec2 oUV;

void main() {
    oUV = iUV;
    gl_Position = ubo.mvp * vec4(iPos, 1.0);
}


// ========== chunk.frag ==========

#version 460

precision highp float;
precision highp int;

layout(location = 0) in VertexData {
  mediump vec2 uv;
  flat mediump float arrayLayer;
  mediump vec3 light;
  mediump float fog;
  mediump vec3 fogColor;

#ifdef SHADOWS_ENABLED
  flat uint shadowEnabled;
  highp vec4 shadowCoord;
#endif
} iVert;

layout(location = 0) out vec4 oFragColor;

layout(binding = 1, set = 1) uniform sampler2DArray uTexture;

#ifdef SHADOWS_ENABLED

layout(binding = 0, set = 2) uniform sampler2DShadow uShadowMap;

#include "shadow.glsl"

#endif

void main() {
  oFragColor = texture(
      uTexture,
      vec3(iVert.uv, iVert.arrayLayer));

#ifdef CUTOUT_LAYER
  if (oFragColor.a < 0.5) {
    discard;
  }
#endif

#ifdef SHADOWS_ENABLED
  if (iVert.shadowEnabled != 0) {
    const float shadow = CalcShadow(
        uShadowMap,
        iVert.shadowCoord);

    oFragColor.rgb *= mix(
        kShadowColor,
        vec3(1.0),
       shadow);
  } else {
    oFragColor.rgb *= kShadowColor;
  }
#endif

  oFragColor.rgb *= iVert.light;

  oFragColor.rgb = mix(
      iVert.fogColor,
      oFragColor.rgb,
      iVert.fog);
}


// ========== chunk.vert ==========

#version 460

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

#ifdef SHADOWS_ENABLED
  flat uint shadowEnabled;
  highp vec4 shadowCoord;
#endif
} oVert;

#include "UniformGameData.glsl"

struct UvRegion {
  vec4 uvRect;
  uint arrayLayer;
};

layout(binding = 0, set = 1) uniform BlockUvBuffer {
  UvRegion regions[512];
} uUv;

#include "cube_uvs.glsl"

const vec3 kFaceNormals[6] = vec3[](
  vec3( 0,  0, -1),
  vec3( 0,  0,  1),
  vec3(-1,  0,  0),
  vec3( 1,  0,  0),
  vec3( 0, -1,  0),
  vec3( 0,  1, 0)
);

void main() {
  uint uvIndex = aFaceIndex * 4u + aCornerIndex;
  vec2 localUV = CUBE_UVS[uvIndex];

  UvRegion region = uUv.regions[aSurfaceId];

  vec2 uvMin = region.uvRect.xy;
  vec2 uvMax = region.uvRect.zw;

  oVert.uv = mix(uvMin, uvMax, localUV);
  oVert.arrayLayer = float(region.arrayLayer);

  vec3 faceNormal = kFaceNormals[aFaceIndex];

  vec3 sunLight = uGameData.ambientColor * aLight.a;
  oVert.light = clamp(aLight.rgb + sunLight, 0.0, 1.0);

  const float GAMMA = 0.6;
  oVert.light = pow(oVert.light, vec3(1.0 / GAMMA));

  vec4 worldPos = vec4(aPos, 1.0);

  float dist = distance(worldPos.xyz, uGameData.cameraPos);

  float fogFactor = exp2(
      -uGameData.fogDensity *
       uGameData.fogDensity *
       dist * dist *
       1.442695
  );

  oVert.fog = clamp(fogFactor, 0.0, 1.0);
  oVert.fogColor = uGameData.ambientColor;

#ifdef SHADOWS_ENABLED
  float lightFacing = dot(faceNormal, -uGameData.lightDir);
  oVert.shadowEnabled = uint(lightFacing > 0.0);

  if (oVert.shadowEnabled != 0) {
    vec3 offsetWorldPos = worldPos.xyz + faceNormal * 0.4;

    vec4 lightSpacePos =
        uGameData.lightProjView * vec4(offsetWorldPos, 1.0);

    oVert.shadowCoord = lightSpacePos / lightSpacePos.w;
    oVert.shadowCoord.xy = oVert.shadowCoord.xy * 0.5 + 0.5;
  }
#endif

  gl_Position = uGameData.projView * worldPos;
}


// ========== chunk_shadow.frag ==========

#version 460

precision highp float;
precision highp int;

layout(binding = 1, set = 1) uniform sampler2DArray uTexture;

#ifdef CUTOUT_LAYER
layout(location = 0) in VertexData {
  mediump vec2 uv;
  mediump flat float arrayLayer;
} iVert;

void main() {
  if (texture(uTexture, vec3(iVert.uv, iVert.arrayLayer)).a < 0.5f) discard;
}
#else 
void main() {
}
#endif


// ========== chunk_shadow.vert ==========

#version 460

precision highp float;
precision highp int;

layout(location = 0) in highp vec3 aPos;
layout(location = 1) in mediump vec4 aLight;
layout(location = 2) in uint aFaceIndex;
layout(location = 3) in uint aCornerIndex;
layout(location = 4) in uint aSurfaceId;

#include "UniformGameData.glsl"

struct UvRegion {
  vec4 uvRect;
  uint arrayLayer;
};

layout(binding = 0, set = 1) uniform BlockUvBuffer {
  UvRegion regions[512];
} uUv;

#ifdef CUTOUT_LAYER
layout(location = 0) out VertexData {
  mediump vec2 uv;
  mediump flat float arrayLayer;
} oVert;

#include "cube_uvs.glsl"
#endif

void main() {
  gl_Position = uGameData.lightProjView * vec4(aPos, 1.0);
#ifdef CUTOUT_LAYER
  uint uvIndex = aFaceIndex * 4u + aCornerIndex;
  vec2 localUV = CUBE_UVS[uvIndex];

  UvRegion region = uUv.regions[aSurfaceId];

  vec2 uvMin = region.uvRect.xy;
  vec2 uvMax = region.uvRect.zw;

  oVert.uv = mix(uvMin, uvMax, localUV);
  oVert.arrayLayer = float(region.arrayLayer);
#endif
}


// ========== crosshair.frag ==========

#version 450

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}


// ========== crosshair.vert ==========

#version 450

vec2 positions[4] = vec2[](
    vec2(-0.02, 0.0),
    vec2( 0.02, 0.0),
    vec2( 0.0, -0.02),
    vec2( 0.0,  0.02)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}


// ========== model.frag ==========

#version 460

precision highp float;
precision highp int;

layout(location = 0) in VertexData {
  mediump vec2 uv;
  mediump vec4 color;
  mediump vec3 light;
  mediump float fog;
  mediump vec3 fogColor;

#ifdef SHADOWS_ENABLED
  flat uint shadowEnabled;
  highp vec4 shadowCoord;
#endif
} iVert;

layout(binding = 0, set = 1) uniform sampler2D uAlbedoTex;

#ifdef SHADOWS_ENABLED

layout(binding = 0, set = 2) uniform sampler2DShadow uShadowMap;

#include "shadow.glsl"

#endif

layout(location = 0) out vec4 oFragColor;

void main() {
  oFragColor = texture(uAlbedoTex, iVert.uv) * iVert.color;

#ifdef SHADOWS_ENABLED
  if (iVert.shadowEnabled != 0) {
    const float shadow = CalcShadow(uShadowMap, iVert.shadowCoord);

    oFragColor.rgb *= mix(kShadowColor, vec3(1.0), shadow);
  } else {
    oFragColor.rgb *= kShadowColor;
  }
#endif

  oFragColor.rgb *= iVert.light;

  oFragColor.rgb = mix(iVert.fogColor, oFragColor.rgb, iVert.fog);
}


// ========== model.vert ==========

#version 460

precision highp float;
precision highp int;

layout(location = 0) in highp vec3 aPos;
layout(location = 1) in mediump vec2 aUv;
layout(location = 2) in highp vec3 aNormal;
layout(location = 3) in vec3 instanceTransformCol0;
layout(location = 4) in vec3 instanceTransformCol1;
layout(location = 5) in vec3 instanceTransformCol2;
layout(location = 6) in vec3 instanceTransformCol3;
layout(location = 7) in vec4 instanceLight;

#include "UniformGameData.glsl"

layout(location = 0) out VertexData {
  mediump vec2 uv;
  mediump vec4 color;
  mediump vec3 light;
  mediump float fog;
  mediump vec3 fogColor;

#ifdef SHADOWS_ENABLED
  flat uint shadowEnabled;
  highp vec4 shadowCoord;
#endif
} oVert;

void main() {
  mat4x3 instanceTransform = mat4x3(
      instanceTransformCol0,
      instanceTransformCol1,
      instanceTransformCol2,
      instanceTransformCol3);

  vec4 worldPos = vec4(instanceTransform * vec4(aPos, 1.0), 1.0);

  gl_Position = uGameData.projView * worldPos;

  oVert.uv = aUv;
  oVert.color = instanceLight;

  vec3 sunLight = uGameData.ambientColor * instanceLight.a;

  oVert.light = clamp(instanceLight.rgb + sunLight, 0.0, 1.0);

  const float GAMMA = 0.6;
  oVert.light = pow(oVert.light, vec3(1.0 / GAMMA));

  float dist = distance(worldPos.xyz, uGameData.cameraPos);

  float fogFactor = exp2(
      -uGameData.fogDensity * uGameData.fogDensity * dist * dist * 1.442695);

  oVert.fog = clamp(fogFactor, 0.0, 1.0);
  oVert.fogColor = uGameData.ambientColor;

#ifdef SHADOWS_ENABLED
  float lightFacing = dot(aNormal, -uGameData.lightDir);

  oVert.shadowEnabled = uint(lightFacing > 0.0);

  if (oVert.shadowEnabled != 0) {
    vec3 offsetWorldPos = worldPos.xyz + aNormal * 0.4;

    vec4 lightSpacePos = uGameData.lightProjView * vec4(offsetWorldPos, 1.0);

    oVert.shadowCoord = lightSpacePos / lightSpacePos.w;

    oVert.shadowCoord.xy = oVert.shadowCoord.xy * 0.5 + 0.5;
  }
#endif
}


// ========== model_shadow.frag ==========

#version 460

precision highp float;
precision highp int;

layout(binding = 0, set = 1) uniform sampler2D uAlbedoTex;

void main() {
}


// ========== model_shadow.vert ==========

#version 460

precision highp float;
precision highp int;

layout(location = 0) in vec3 aPos;
//layout(location = 1) in vec2 aUv;
//layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 instanceTransformCol0;
layout(location = 4) in vec3 instanceTransformCol1;
layout(location = 5) in vec3 instanceTransformCol2;
layout(location = 6) in vec3 instanceTransformCol3;
//layout(location = 7) in vec4 instanceColor;

#include "UniformGameData.glsl"

void main() {
  mat4x3 instanceTransform = mat4x3(instanceTransformCol0, instanceTransformCol1,
                                    instanceTransformCol2, instanceTransformCol3);
  vec4 worldPos = vec4(instanceTransform * vec4(aPos, 1.0), 1.0);

  gl_Position = uGameData.lightProjView * worldPos;
}


// ========== skybox.frag ==========

#version 460

precision highp float;
precision highp int;

layout(location = 0) in Vertex {
  mediump vec3 uv;
} iVert;

layout(binding = 0, set = 1) uniform samplerCube uSkybox;

layout(location = 0) out vec4 oFragColor;

void main() {
  oFragColor = texture(uSkybox, iVert.uv);
}


// ========== skybox.vert ==========

#version 460

precision highp float;
precision highp int;

layout(location = 0) in vec3 aPos;

layout(binding = 0, set = 0) uniform UniformGameData {
  mat4 projView;
  mat4 proj;
  mat4 view;
  vec3 cameraPos;
  vec3 cameraDir;
} uGameData;

layout(location = 0) out Vertex {
  mediump vec3 uv;
} oVert;

void main() {
  mat4 view = mat4(mat3(uGameData.view));
  gl_Position = uGameData.proj * view * vec4(aPos, 1.0f);
  gl_Position.z = gl_Position.w * (1.0 - 1e-5);
  oVert.uv = aPos;
}


// ========== ui.frag ==========

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


// ========== ui.vert ==========

#version 460

precision highp float;
precision highp int;

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aSize;
layout(location = 2) in vec4 aUvRect;
layout(location = 3) in vec4 aColor;
layout(location = 4) in float aRadius;
layout(location = 5) in float aBorderWidth;
layout(location = 6) in uint aType;

layout(push_constant) uniform PushData {
  vec2 ndcScale;
} pc;

layout(location = 0) out VertexData {
  vec2 uv;
  mediump vec4 color;
  vec2 local;
  flat vec2 halfSize;
  flat float radius;
  flat float borderWidth;
  flat uint type;
} oVert;

const vec2 kCorners[6] = vec2[](
  vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0),
  vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(1.0, 0.0)
);

void main() {
  vec2 c = kCorners[gl_VertexIndex];

  gl_Position = vec4((aPos + c * aSize) * pc.ndcScale - 1.0, 0.0, 1.0);

  oVert.local      = (c - 0.5) * aSize;
  oVert.uv         = mix(aUvRect.xy, aUvRect.zw, c);
  oVert.color      = aColor;
  oVert.halfSize   = aSize * 0.5;
  oVert.radius     = aRadius;
  oVert.borderWidth = aBorderWidth;
  oVert.type       = aType;
}
