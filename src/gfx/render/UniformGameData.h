#pragma once

#include "glm/glm.hpp"

namespace gfx {

struct UniformGameData {
  glm::mat4 projView;               // offset 0,   size 64
  glm::mat4 proj;                   // offset 64,  size 64
  glm::mat4 view;                   // offset 128, size 64
  alignas(16) glm::vec3 cameraPos;  // offset 192, vec3
  float _pad0 = 0.0f;
  alignas(16) glm::vec3 cameraDir;  // offset 208
  float _pad1 = 0.0f;
};

}  // namespace gfx