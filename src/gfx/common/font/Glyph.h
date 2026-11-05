#pragma once

#include <glm/glm.hpp>

#include "../../UvRegion.h"
#include "../texture/MaterialManager.h"

namespace gfx {

struct GlyphMetrics {
  glm::ivec2 size = {0, 0};
  glm::ivec2 bearing = {0, 0};
  float advance = 0.0f;
};

struct GlyphSlot {
  MaterialId materialId = kInvalidMaterialId;
  UvRegion region;
};

struct Glyph {
  GlyphMetrics metrics;
  GlyphSlot slot;
};
}  // namespace gfx