#pragma once

#include <stdint.h>

#include <optional>
#include <string>
#include <variant>

#include "../../UvRegion.h"
#include "BlockAnimation.h"

namespace gfx {
using BlockSurface = std::variant<UvRegion, BlockAnimation>;
}  // namespace gfx