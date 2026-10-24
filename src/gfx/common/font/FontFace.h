#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace gfx {
class FontFace {
 public:
  FontFace();
 private:
  std::vector<std::byte> buffer_;
};
}  // namespace gfx
