#include "Font.h"

namespace gfx {
Font::Font(FontFace&& fontFace, std::reference_wrapper<GlyphAtlas> glyphAtlas)
	: fontFace_(std::move(fontFace)), glyphAtlas_(glyphAtlas) {}

Glyph Font::GetGlyph(char32_t c) {
  
}
}  // namespace gfx