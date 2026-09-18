#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <limits>

namespace gfx {

using RenderGroupId = uint32_t;

class RenderGroupRegistry {
 public:
  static constexpr RenderGroupId kInvalid = std::numeric_limits<RenderGroupId>::max();

  [[nodiscard]] RenderGroupId RegisterGroup(std::string_view name) {
    auto it = groups_.find(name.data());

    if (it != groups_.end()) {
      return it->second;
    }

    RenderGroupId id = static_cast<RenderGroupId>(names_.size());

    names_.emplace_back(name);
    groups_.emplace(names_.back(), id);

    return id;
  }

  [[nodiscard]] RenderGroupId Resolve(std::string_view name) const {
    auto it = groups_.find(name.data());

    if (it == groups_.end()) {
      return kInvalid;
    }

    return it->second;
  }

  [[nodiscard]] std::string_view name(RenderGroupId id) const { return names_.at(id); }

 private:
  std::unordered_map<std::string, RenderGroupId> groups_;
  std::vector<std::string> names_;
};

}  // namespace gfx