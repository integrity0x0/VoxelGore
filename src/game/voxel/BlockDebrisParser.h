#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string_view>

#include "BlockDebrisConfig.h"

namespace gm {

class BlockDebrisParser {
 public:
  static BlockDebrisConfig parseDebrisConfig(const nlohmann::json& j);
  static std::optional<BlockDebrisConfig> parseFromBlockJson(const nlohmann::json& j);

 private:
  static BlockDebrisConfig::Face parseFace(const std::string& faceStr);
};

}  // namespace gm