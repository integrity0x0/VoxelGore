#pragma once

#include <stdint.h>

#include <glm/glm.hpp>

namespace gfx {
struct BillboardInstance {
  glm::vec3 pos;
  float rotation;
  glm::vec2 size;
  glm::vec4 uvMinMax;
  float layer;
  glm::vec4 color;

  BillboardInstance() = default;
};
}  // namespace gfx