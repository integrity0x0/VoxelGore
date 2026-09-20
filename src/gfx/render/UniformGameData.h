#pragma once

#include "glm/glm.hpp"

namespace gfx {

struct UniformGameData {
  glm::mat4 projView;  // 0
  glm::mat4 proj;      // 64
  glm::mat4 view;      // 128

  alignas(16) glm::vec3 cameraPos;  // 192
  float _pad0;               // 204

  alignas(16) glm::vec3 cameraDir;  // 208
  float _pad1;               // 220

  alignas(16) glm::vec3 ambientColor;  // 224
  float fogDensity;             // 236

  alignas(16) glm::mat4 lightProjView;  // 240

  alignas(16) glm::vec3 lightDir;  // 304
  float _pad2;              // 316
};

}  // namespace gfx