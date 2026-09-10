#pragma once

#include <stdint.h>

#include <optional>
#include <string>
#include <variant>

#include "../UvRegion.h"
#include "Animation.h"

namespace gfx::block {
using Surface = std::variant<UvRegion, Animation>;
}  // namespace gfx::block