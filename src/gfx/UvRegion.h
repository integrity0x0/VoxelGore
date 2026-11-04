#pragma once

#include "glm/glm.hpp"

namespace gfx {
struct UvRegion {
  glm::vec2 min = glm::vec2(0.0f);
  glm::vec2 max = glm::vec2(1.0f);
  uint32_t arrayLayer = 0;
};
}  // namespace gfx