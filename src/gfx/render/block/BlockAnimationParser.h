#pragma once

#include <string>

#include "../../../util/pathUtils.h"
#include "../../common/texture/Atlas.h"
#include "BlockAnimation.h"
#include "../../../../include/nlohmann/json.hpp"

namespace gfx {

class AnimationParser {
 public:
  AnimationParser() = delete;
  [[nodiscard]] static BlockAnimation Parse(const std::string& jsonPath, Atlas& atlas);
};

}  // namespace gfx