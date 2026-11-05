#pragma once

#include <unordered_map>
#include <functional>

#include "../../UvRegion.h"
#include "GlyphAtlas.h"
#include "FontFace.h"
#include "FontStyle.h"
#include "../texture/MaterialManager.h"
#include "Glyph.h"

namespace gfx {
class Font {
 public:
  Font(FontFace&& fontFace, std::reference_wrapper<GlyphAtlas> glyphAtlas);

  [[nodiscard]] Glyph GetGlyph(char32_t c);
 private:
  FontFace fontFace_;
  std::reference_wrapper<GlyphAtlas> glyphAtlas_;
  std::unordered_map<char32_t, Glyph> glyphs_;
};
}  // namespace gfx
