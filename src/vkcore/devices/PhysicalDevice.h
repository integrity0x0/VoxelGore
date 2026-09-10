#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "../instances/Instance.h"
#include "QueueFamilyIndices.h"

namespace vkcore {
class PhysicalDevice {
 public:
  PhysicalDevice(const Instance& instance, VkPhysicalDevice physicalDevice);
  PhysicalDevice() = default;
  bool supportsExtensions(const std::vector<std::string>& required) const;
  bool supportsQueueFamilies(const std::vector<VkQueueFlags>& required) const;

  const VkPhysicalDeviceProperties& getProperties() const noexcept { return properties; }
  const VkPhysicalDeviceFeatures& getFeatures() const noexcept { return features; }

  const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const { return memoryProperties; }
  int32_t score() const;
  VkPhysicalDevice handle() const { return physicalDevice; }
  QueueFamilyIndices getQueueFamilyIndices(VkSurfaceKHR surface) const;

  std::optional<uint32_t> findMemoryType(VkMemoryPropertyFlags requiredFlags,
                                         uint32_t memoryTypeBits) const {
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
      if ((memoryTypeBits & (1 << i)) &&
          (memoryProperties.memoryTypes[i].propertyFlags & requiredFlags) == requiredFlags) {
        return i;
      }
    }

    return std::nullopt;
  }

  bool supportsExtension(const std::string& required) const {
    return std::find(supportedExtensions.begin(), supportedExtensions.end(), required) !=
           supportedExtensions.end();
  }

  ~PhysicalDevice() = default;

 private:
  const Instance* instance;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memoryProperties;
  std::vector<std::string> supportedExtensions;
};

extern std::unique_ptr<PhysicalDevice> pick(const Instance& instance,
                                            const std::vector<std::string>& requiredExtensions,
                                            const std::vector<VkQueueFlags>& requiredFamilies);
}  // namespace vkcore