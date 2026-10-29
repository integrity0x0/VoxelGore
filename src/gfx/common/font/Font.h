#pragma once

#include <unordered_map>
#include <functional>

#include "GlyphAtlas.h"
#include "FontFace.h"

namespace gfx {
class Font {
 public:
  Font();
 private:
  std::reference_wrapper<GlyphAtlas> glyphAtlas_;
  FontFace fontFace_;
};
}  // namespace gfx
