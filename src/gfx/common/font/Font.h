#pragma once

#include <functional>
#include <unordered_map>

#include "../../../util/hashers.h"
#include "FontFace.h"
#include "FontStyle.h"
#include "Glyph.h"
#include "GlyphAtlas.h"

namespace gfx {
class Font {
 public:
  Font(const std::string& familyName, FontStyle style, FontFace&& fontFace,
       std::reference_wrapper<GlyphAtlas> glyphAtlas);

  [[nodiscard]] const Glyph& GetGlyph(char32_t c, uint32_t pixelSize);

  [[nodiscard]] const FontMetrics& GetMetrics(uint32_t pixelSize);

  [[nodiscard]] FontStyle GetStyle() const { return style_; }

  [[nodiscard]] const std::string& GetFamilyName() const { return familyName_; }

 private:
  struct GlyphKey {
    char32_t codepoint;
    uint32_t pixelSize;

    [[nodiscard]] bool operator==(const GlyphKey&) const = default;
  };

  struct GlyphKeyHash {
    [[nodiscard]] size_t operator()(const GlyphKey& key) const noexcept {
      size_t seed = 0;
      util::HashCombine(seed, key.codepoint);
      util::HashCombine(seed, key.pixelSize);
      return seed;
    }
  };

 private:
  std::string familyName_;
  FontStyle style_;
  FontFace face_;
  std::reference_wrapper<GlyphAtlas> glyphAtlas_;
  std::unordered_map<GlyphKey, Glyph, GlyphKeyHash> glyphs_;
  std::unordered_map<uint32_t, FontMetrics> metrics_;
};
}  // namespace gfx