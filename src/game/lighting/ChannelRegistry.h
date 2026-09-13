#pragma once

#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "ChannelDefinition.h"
#include "../../util/hashers.h"

namespace gm::lighting {

using ChannelId = uint16_t;
static constexpr ChannelId kInvalidChannelId = std::numeric_limits<ChannelId>::max();

class ChannelRegistry {
 public:
  explicit ChannelRegistry(ChannelDefinition&& sunDefinition);

  [[nodiscard]] ChannelId Require(std::string_view id);
  [[nodiscard]] ChannelId Register(ChannelDefinition definition);

  [[nodiscard]] ChannelId Find(std::string_view id) const;
  [[nodiscard]] const ChannelDefinition* Get(ChannelId id) const;
  [[nodiscard]] const ChannelDefinition* Get(std::string_view id) const;

 private:
  std::unordered_map<std::string, ChannelId, util::StringHash, std::equal_to<>> ids_;

  std::vector<ChannelDefinition> definitions_;
};

}  // namespace gm::lighting