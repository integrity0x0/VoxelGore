#pragma once 

#include <stdint.h>

namespace gfx {
struct Settings {
  bool shadowsEnable;
  uint32_t anisotropyLevel;
};
}  // namespace gfx