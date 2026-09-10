#pragma once
#include <vulkan/vulkan.h>

#include <optional>

namespace vkcore {
struct QueueFamilyIndices {
  std::optional<uint32_t> graphics;
  std::optional<uint32_t> present;
  bool isComplete() const { return graphics.has_value() && present.has_value(); }
};
}  // namespace vkcore
