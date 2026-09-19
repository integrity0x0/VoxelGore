#pragma once

#include <stdint.h>

namespace gfx {
struct Settings {
  uint32_t anisotropyLevel = 4;
  bool enableShadows = false;
};
}  // namespace gfx