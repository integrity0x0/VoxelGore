#pragma once

#include <glm/glm.hpp>

namespace gfx {
struct UiQuadInstance {
  glm::vec2 pos;
  glm::vec2 size;
  glm::vec4 uvRect;
  glm::vec4 color;
  float radius;
};
}  // namespace gfx