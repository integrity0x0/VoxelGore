#pragma once

#include <string>

#include "../../util/pathUtils.h"
#include "../texture/Atlas.h"
#include "Animation.h"
#include "nlohmann/json.hpp"

namespace gfx::block {

class AnimationParser {
 public:
  [[nodiscard]] static Animation parse(const std::string& jsonPath, Atlas& atlas);
};

}  // namespace gfx::block