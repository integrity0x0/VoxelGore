#pragma once

#include "../RenderLayers.h"
#include "glm/glm.hpp"

namespace gfx {
struct Particle {
  enum class AtlasType { General, Terrain };

  AtlasType atlasType = AtlasType::General;

  RenderLayer renderLayer = RenderLayer::Solid;

  glm::vec3 pos;
  glm::vec3 velocity;

  glm::vec3 acceleration;

  float rotation;
  float angularVelocity;

  glm::vec2 size;

  glm::vec4 uvMinMax;
  float layer;

  bool ignoreLighting;

  bool collision;
  bool settled_ = false;

  glm::vec4 color;

  float life;
  float maxLife;

  float bounceFactor = 0.4f;

  size_t variantId;
};
}  // namespace gfx