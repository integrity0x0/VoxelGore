#pragma once

#include <unordered_map>
#include <functional>

#include "GlyphAtlas.h"
#include "FontFace.h"
#include "FontStyle.h"
#include "../texture/MaterialManager.h"

namespace gfx {
struct GlyphRenderInfo {
  GlyphInfo info;
  UvRegion uvRegion;
};

class Font {
 public:
  Font(FontFace&& fontFace, std::reference_wrapper<GlyphAtlas> glyphAtlas);

  [[nodiscard]] std::optional<GlyphRenderInfo> GetGlyph(char32_t c);
 private:
  FontFace fontFace_;
  std::reference_wrapper<GlyphAtlas> glyphAtlas_;
  std::unordered_map<char32_t, GlyphRenderInfo> glyphs_;
};
}  // namespace gfx
