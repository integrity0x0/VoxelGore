#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

#include "FreeTypeLibrary.h"
#include "Glyph.h"

namespace gfx {

struct FontMetrics {
  float ascent = 0.0f;
  float descent = 0.0f;
  float lineHeight = 0.0f;
};

struct RasterizedGlyph {
  GlyphMetrics metrics;
  std::vector<std::byte> pixels;
};

class FontFace {
 public:
  FontFace(const FreeTypeLibrary& lib, std::vector<std::byte>&& bytes);

  [[nodiscard]] uint32_t GetCurrentWidth() const { return currentWidth_; }
  [[nodiscard]] uint32_t GetCurrentHeight() const { return currentHeight_; }

  [[nodiscard]] RasterizedGlyph Rasterize(char32_t c, uint32_t w = 16, uint32_t h = 0);

  [[nodiscard]] FontMetrics GetMetrics() const;

  void SetSize(uint32_t w, uint32_t h = 0);

 private:
  struct FtFaceDeleter {
    void operator()(std::remove_pointer_t<FT_Face>* face) const noexcept { FT_Done_Face(face); }
  };

  using FtFacePtr = std::unique_ptr<std::remove_pointer_t<FT_Face>, FtFaceDeleter>;

  [[nodiscard]] FtFacePtr CreateFace(const FreeTypeLibrary& lib);


 private:
  std::vector<std::byte> bytes_;
  FtFacePtr face_;
  uint32_t currentWidth_ = 0;
  uint32_t currentHeight_ = 0;
};
}  // namespace gfx