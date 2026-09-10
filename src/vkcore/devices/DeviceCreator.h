#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "../instances/Instance.h"
#include "Device.h"

namespace vkcore {

class DeviceCreator {
 public:
  DeviceCreator(const PhysicalDevice& physicalDevice, const Instance& instance)
      : physicalDevice_(physicalDevice), instance_(&instance) {}

  DeviceCreator& addQueue(uint32_t familyIndex, float priority = 1.0f) {
    queueRequests_.push_back({familyIndex, priority});
    return *this;
  }

  DeviceCreator& addQueues(uint32_t familyIndex, uint32_t queueCount, float priority = 1.0f) {
    for (uint32_t i = 0; i < queueCount; ++i) {
      addQueue(familyIndex, priority);
    }
    return *this;
  }

  DeviceCreator& setQueues(std::vector<std::pair<uint32_t, float>> queues) {
    queueRequests_ = std::move(queues);
    return *this;
  }

  DeviceCreator& addExtension(const std::string& extensionName) {
    extensions_.push_back(extensionName);
    return *this;
  }

  DeviceCreator& addExtensions(const std::vector<std::string>& extensionNames) {
    extensions_.insert(extensions_.end(), extensionNames.begin(), extensionNames.end());
    return *this;
  }

  DeviceCreator& addValidationLayer(const std::string& layerName) {
    validationLayers_.push_back(layerName);
    return *this;
  }

  DeviceCreator& addValidationLayers(const std::vector<std::string>& layerNames) {
    validationLayers_.insert(validationLayers_.end(), layerNames.begin(), layerNames.end());
    return *this;
  }

  DeviceCreator& setEnabledFeatures(const VkPhysicalDeviceFeatures& features) {
    enabledFeatures_ = features;
    return *this;
  }

  const PhysicalDevice& getPhysicalDevice() const noexcept { return physicalDevice_; }

  const Instance* getInstance() const noexcept { return instance_; }

  const std::vector<std::pair<uint32_t, float>>& getQueueRequests() const noexcept {
    return queueRequests_;
  }

  const std::vector<std::string>& getExtensions() const noexcept { return extensions_; }

  const std::vector<std::string>& getValidationLayers() const noexcept { return validationLayers_; }

  const VkPhysicalDeviceFeatures& getEnabledFeatures() const noexcept { return enabledFeatures_; }

  Device createDevice();

 private:
  const PhysicalDevice physicalDevice_;
  const Instance* instance_;

  std::vector<std::pair<uint32_t, float>> queueRequests_;

  std::vector<std::string> extensions_;
  std::vector<std::string> validationLayers_;

  VkPhysicalDeviceFeatures enabledFeatures_{};
};

}  // namespace vkcore