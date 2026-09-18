#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "../common/vulkanFunctions.h"
#include "DeviceQueue.h"
#include "PhysicalDevice.h"

namespace vkcore {

class DeviceCreator;

class Device {
 public:
  Device() = default;

  Device(Device&& other) noexcept;
  Device& operator=(Device&& other) noexcept;

  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;

  ~Device() = default;

  VkDevice handle() const noexcept { return device_.get(); }
  const DeviceDispatchTable& dispatchTable() const noexcept { return dispatchTable_; }
  const PhysicalDevice& getPhysicalDevice() const noexcept { return physicalDevice_; }
  const std::vector<std::string>& getEnabledExtensions() const noexcept {
    return enabledExtensions_;
  }
  const VkPhysicalDeviceFeatures& getFeatures() const noexcept { return features_; }

  const std::vector<DeviceQueue>& getQueues() const noexcept { return queues_; }

  std::optional<DeviceQueue> findQueue(VkQueueFlags flags) const {
    for (const auto& q : queues_) {
      if ((q.flags() & flags) == flags) return q;
    }
    return std::nullopt;
  }

  bool isExtensionEnabled(const std::string& ext) const noexcept {
    return std::find(enabledExtensions_.begin(), enabledExtensions_.end(), ext) !=
           enabledExtensions_.end();
  }

  void WaitIdle() const { dispatchTable_.vkDeviceWaitIdle(device_.get()); }

 private:
  Device(UniqueDevice device, DeviceDispatchTable dispatchTable,
         std::vector<std::string> enabledExtensions, VkPhysicalDeviceFeatures features,
         std::vector<DeviceQueue> queues, PhysicalDevice physicalDevice)
      : device_(std::move(device)),
        dispatchTable_(dispatchTable),
        enabledExtensions_(std::move(enabledExtensions)),
        features_(features),
        queues_(std::move(queues)),
        physicalDevice_(std::move(physicalDevice)) {
    rebindQueues();
  }

  friend class DeviceCreator;

  void rebindQueues() {
    for (auto& q : queues_) {
      q.RebindDispatchTable(dispatchTable_);
    }
  }

  UniqueDevice device_;
  DeviceDispatchTable dispatchTable_;
  std::vector<std::string> enabledExtensions_;
  VkPhysicalDeviceFeatures features_;
  std::vector<DeviceQueue> queues_;
  PhysicalDevice physicalDevice_;
};

}  // namespace vkcore