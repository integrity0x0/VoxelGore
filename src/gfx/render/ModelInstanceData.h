#pragma once

#include <glm/glm.hpp>

namespace gfx {
struct ModelInstanceData {
  glm::mat4x3 transform;
  glm::vec4 color;
};
}  // namespace gfx
