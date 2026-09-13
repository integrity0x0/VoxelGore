#pragma once

#include <optional>
#include <string_view>

#include "ChannelDefinition.h"

namespace gm::lighting {
class ChannelParser {
 public:
  ChannelParser() = delete;

  [[nodiscard]] static std::optional<ChannelDefinition> Parse(std::string_view path);
};
}  // namespace gm::lighting
