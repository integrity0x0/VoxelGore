const vec3 kShadowColor = vec3(0.28);
const float kShadowPcfRadius = 1.0;

float SampleShadow(sampler2DShadow shadowMap, vec3 shadowCoord) {
  return texture(shadowMap, shadowCoord);
}

float SampleShadowPcf(sampler2DShadow shadowMap, vec3 shadowCoord) {
  const vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

  float shadow = 0.0;

  for (int y = -1; y <= 1; ++y) {
    for (int x = -1; x <= 1; ++x) {
      const vec2 offset = vec2(x, y) * texelSize * kShadowPcfRadius;

      shadow += SampleShadow(
          shadowMap,
          vec3(shadowCoord.xy + offset, shadowCoord.z));
    }
  }

  return shadow / 9.0f;
}

float CalcShadow(sampler2DShadow shadowMap, vec4 shadowCoord) {
  return SampleShadowPcf(shadowMap, shadowCoord.xyz);
}