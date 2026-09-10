#pragma once

#include <glm/glm.hpp>
#include <string>

#include "../voxel/Block.h"

namespace gm {

struct RenderComponent {
  enum class Type { Model, Billboard };
  Type type;
  enum class RenderLayer { Solid, Cutout, Translucent, Count };
  RenderLayer renderLayer = RenderLayer::Solid;
  std::string resource;
  glm::vec2 pivot;
  glm::vec2 billboardSize;
  bool ignoreLighting = false;
  bool ignoreHurtColor = false;
};
}  // namespace gm