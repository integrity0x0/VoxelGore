#include "Font.h"

namespace gfx {
Font::Font(const std::string& familyName, FontStyle style, FontFace&& fontFace,
           std::reference_wrapper<GlyphAtlas> glyphAtlas)
    : familyName_(familyName), style_(style), face_(std::move(fontFace)), glyphAtlas_(glyphAtlas)
{}

const Glyph& Font::GetGlyph(char32_t c, uint32_t pixelSize) {
  const GlyphKey key{c, pixelSize};
  if (auto it = glyphs_.find(key); it != glyphs_.end()) return it->second;

  

  RasterizedGlyph raster = face_.Rasterize(c, pixelSize);

  Glyph glyph;
  glyph.metrics = raster.metrics;

  if (glyph.metrics.size.x > 0 && glyph.metrics.size.y > 0) {
    if (auto slot = glyphAtlas_.get().AddGlyph(glyph.metrics.size, raster.pixels))
      glyph.slot = *slot;
  }

  return glyphs_.emplace(key, glyph).first->second;
}

const FontMetrics& Font::GetMetrics(uint32_t pixelSize) {
  if (auto it = metrics_.find(pixelSize); it != metrics_.end()) return it->second;
  face_.SetSize(pixelSize);
  return metrics_.emplace(pixelSize, face_.GetMetrics()).first->second;
}
}  // namespace gfx