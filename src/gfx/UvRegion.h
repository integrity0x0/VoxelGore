#pragma once

#include "glm/glm.hpp"

namespace gfx {
struct UvRegion {
  glm::vec2 min;
  glm::vec2 max;
  uint32_t arrayLayer;
};
}  // namespace gfx