#pragma once

#include <glm/glm.hpp>

namespace gm {
struct BleedComponent {
  bool hurted = false;
  float damage = 0.0f;
  glm::vec3 pos = {};
  glm::vec3 normal = {};
};
}  // namespace gm