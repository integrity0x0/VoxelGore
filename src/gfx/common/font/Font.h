#pragma once

#include <unordered_map>
#include <functional>

#include "GlyphAtlas.h"
#include "FontFace.h"
#include "../texture/MaterialManager.h"

namespace gfx {
enum class FontWeight : uint16_t {
  Regular = 400,
  Medium = 500
};

class Font {
 public:
  Font();
 private:
  std::reference_wrapper<GlyphAtlas> glyphAtlas_;
  FontFace fontFace_;
};
}  // namespace gfx
