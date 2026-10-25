#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <memory>
#include <type_traits>

namespace gfx {

class FreeTypeLibrary {
 public:
  FreeTypeLibrary();

  [[nodiscard]] FT_Library GetHandle() const noexcept { return handle_.get(); }

 private:
  struct FtLibraryDeleter {
    void operator()(std::remove_pointer_t<FT_Library>* lib) const noexcept {
      FT_Done_FreeType(lib);
    }
  };

  using FtLibraryPtr = std::unique_ptr<std::remove_pointer_t<FT_Library>, FtLibraryDeleter>;

  [[nodiscard]] static FtLibraryPtr InitLib();

 private:
  FtLibraryPtr handle_;
};
}  // namespace gfx