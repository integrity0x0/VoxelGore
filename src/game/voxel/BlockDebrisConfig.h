#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace gm {

struct BlockDebrisConfig {
  BlockDebrisConfig() = default;

  uint32_t count = 32u;

  glm::vec2 size = {0.15f, 0.15f};

  glm::vec3 velocityMin = {-2.0f, 1.0f, -2.0f};

  glm::vec3 velocityMax = {2.0f, 4.0f, 2.0f};

  glm::vec3 acceleration = {0.0f, -10.0f, 0.0f};

  glm::vec2 lifetime = {1.0f, 2.0f};

  glm::vec2 rotationSpeed = {-8.0f, 8.0f};

  float bounce = 0.2f;

  float friction = 0.8f;

  enum class Face { Random, Top, Bottom, Side };

  Face face = Face::Random;
};

}  // namespace gm