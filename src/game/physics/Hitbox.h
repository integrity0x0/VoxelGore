#pragma once
#include <glm/glm.hpp>

namespace gm {
struct Hitbox {
  glm::vec3 pos;
  glm::vec3 size;
  glm::vec3 vel;
  bool grounded;
};
}  // namespace gm