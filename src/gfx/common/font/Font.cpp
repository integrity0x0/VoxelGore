#include "Font.h"

namespace gfx {
Font::Font(FontFace&& fontFace, std::reference_wrapper<GlyphAtlas> glyphAtlas)
	: fontFace_(std::move(fontFace)), glyphAtlas_(glyphAtlas) {
  
}
}  // namespace gfx