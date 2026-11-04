#pragma once

#include <unordered_map>
#include <memory>

#include "GlyphAtlas.h"
#include "FontFamily.h"

namespace gfx {
class FontManager {
 public:
  FontManager(MaterialManager& materialManager);
private:
  GlyphAtlas atlas_;
  std::unordered_map<std::string, gfx::FontFamily> families_;
};
}  // namespace gfx
