#pragma once

#include <unordered_map>
#include <memory>

#include "GlyphAtlas.h"
#include "Font.h"

namespace gfx {
class FontManager {
 public:
 private:
  GlyphAtlas atlas_;
  std::unordered_map<std::string, gfx::Font> fonts_;
};
}  // namespace gfx
