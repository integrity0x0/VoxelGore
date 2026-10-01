#pragma once

#include <glm/glm.hpp>

namespace gfx {
struct UiVertex {
  glm::vec2 pos;
  glm::vec2 uv;
  glm::vec4 color;
};
}  // namespace gfx