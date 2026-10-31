#include "FontFace.h"

#include <cstring>
#include <stdexcept>
#include <string>
#include <cassert>

namespace gfx {
FontFace::FontFace(const FreeTypeLibrary& lib, std::vector<std::byte>&& bytes)
    : bytes_(std::move(bytes)), face_(CreateFace(lib)) {
  assert(face_);
}

FontFace::FtFacePtr FontFace::CreateFace(const FreeTypeLibrary& lib) {
  FT_Face face = nullptr;
  if (FT_Error e =
          FT_New_Memory_Face(lib.GetHandle(), reinterpret_cast<const FT_Byte*>(bytes_.data()),
                             static_cast<FT_Long>(bytes_.size()), 0, &face)) {
    throw std::runtime_error("Failed to create FT_Face. Error code: " + std::to_string(e));
  }
  return FtFacePtr(face);
}

void FontFace::SetSize(uint32_t w, uint32_t h) {
  if (FT_Error e = FT_Set_Pixel_Sizes(face_.get(), w, h)) {
    throw std::runtime_error("Failed to set pixel size. Error code: " + std::to_string(e));
  }
  currentWidth_ = w;
  currentHeight_ = h;
}

GlyphData FontFace::GetGlyph(char32_t c) {
  FT_Face face = face_.get();

  FT_UInt index = FT_Get_Char_Index(face, static_cast<FT_ULong>(c));
   
  if (FT_Error e = FT_Load_Glyph(face, index, FT_LOAD_DEFAULT)) {
    throw std::runtime_error("Failed to load glyph U+" + std::to_string(static_cast<uint32_t>(c)) +
                             ". Error code: " + std::to_string(e));
  }

  if (FT_Error e = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
    throw std::runtime_error("Failed to render glyph U+" +
                             std::to_string(static_cast<uint32_t>(c)) +
                             ". Error code: " + std::to_string(e));
  }

  const FT_GlyphSlot slot = face->glyph;
  const FT_Bitmap& bmp = slot->bitmap;

  GlyphData result = {};
  result.width = static_cast<int32_t>(bmp.width);
  result.height = static_cast<int32_t>(bmp.rows);
  result.bearingX = slot->bitmap_left;
  result.bearingY = slot->bitmap_top;
  result.advance = static_cast<float>(slot->advance.x) / 64.0f;

  result.pixels.resize(static_cast<size_t>(bmp.width) * bmp.rows);

  for (uint32_t row = 0; row < bmp.rows; ++row) {
    std::memcpy(result.pixels.data() + static_cast<size_t>(row) * bmp.width,
                bmp.buffer + static_cast<ptrdiff_t>(row) * bmp.pitch, bmp.width);
  }

  return result;
}
}  // namespace gfx