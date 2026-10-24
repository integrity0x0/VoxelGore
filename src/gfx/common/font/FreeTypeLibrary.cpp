#include "FreeTypeLibrary.h"

#include <stdexcept>
#include <string>

namespace gfx {
FT_Library FreeTypeLibrary::InitLib() {
  FT_Library raw = nullptr;
  
  if (FT_Error e = FT_Init_FreeType(&raw)) {
    throw std::runtime_error("Failed to initialize FT_Library. Error code: " + std::to_string(e));
  }

  return raw;
}
}  // namespace gfx