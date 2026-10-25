#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

#include "FreeTypeLibrary.h"

namespace gfx {
struct GlyphData {
  int32_t width, height;
  int32_t bearingX, bearingY;
  float advance;
  std::vector<std::byte> pixels;
};

class FontFace {
 public:
  FontFace(const FreeTypeLibrary& lib, std::vector<std::byte>&& bytes);

  FontFace(const FontFace&) = delete;
  FontFace& operator=(const FontFace&) = delete;
  FontFace(FontFace&&) noexcept = default;
  FontFace& operator=(FontFace&&) = delete;

  void SetSize(uint32_t w, uint32_t h);
  void SetSize(uint32_t w) { SetSize(w, 0); }

  [[nodiscard]] uint32_t GetCurrentWidth() const { return currentWidth_; }
  [[nodiscard]] uint32_t GetCurrentHeight() const { return currentHeight_; }

  [[nodiscard]] GlyphData GetGlyph(char32_t c);

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