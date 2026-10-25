#include "FreeTypeLibrary.h"

#include <stdexcept>
#include <string>

namespace gfx {
FreeTypeLibrary::FreeTypeLibrary() : handle_(InitLib()) {}

FreeTypeLibrary::FtLibraryPtr FreeTypeLibrary::InitLib() {
  FT_Library raw = nullptr;

  if (FT_Error e = FT_Init_FreeType(&raw)) {
    throw std::runtime_error("Failed to initialize FT_Library. Error code: " + std::to_string(e));
  }

  return FtLibraryPtr(raw);
}
}  // namespace gfx