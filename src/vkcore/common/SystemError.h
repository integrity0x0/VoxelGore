#pragma once
#include <vulkan/vulkan.h>

#include <stdexcept>
#include <string>
#include <system_error>

namespace vkcore {

class VulkanErrorCategory final : public std::error_category {
 public:
  const char* name() const noexcept override { return "vulkan"; }

  std::string message(int ev) const override { return "VkResult(" + std::to_string(ev) + ")"; }
};

inline const std::error_category& vulkan_category() {
  static VulkanErrorCategory instance;
  return instance;
}

inline std::error_code make_error_code(VkResult result) {
  return {static_cast<int>(result), vulkan_category()};
}

class SystemError final : public std::system_error {
 public:
  SystemError(VkResult result, const std::string& what_arg)
      : std::system_error(make_error_code(result), what_arg) {}

  VkResult result() const noexcept { return static_cast<VkResult>(code().value()); }

  static void Check(VkResult result, const std::string& what_arg) {
    if (result != VK_SUCCESS) throw SystemError(result, what_arg);
  }
};

}  // namespace vkcore