#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

namespace gfx {

class FreeTypeLibrary {
 public:
  FreeTypeLibrary() : handle_(InitLib()) {}

  [[nodiscard]] FT_Library GetHandle() const { return handle_; }

  ~FreeTypeLibrary() { 
    if (handle_) FT_Done_FreeType(handle_); 
  }

 private:
  [[nodiscard]] FT_Library InitLib();

 private:
  FT_Library handle_;
};
}  // namespace gfx