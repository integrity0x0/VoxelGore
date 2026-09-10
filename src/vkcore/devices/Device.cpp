#include "Device.h"

namespace vkcore {

Device::Device(Device&& other) noexcept
    : device_(std::move(other.device_)),
      dispatchTable_(other.dispatchTable_),
      enabledExtensions_(std::move(other.enabledExtensions_)),
      features_(other.features_),
      queues_(std::move(other.queues_)),
      physicalDevice_(std::move(other.physicalDevice_)) {
  rebindQueues();
}

Device& Device::operator=(Device&& other) noexcept {
  if (this == &other) return *this;

  device_ = std::move(other.device_);
  dispatchTable_ = other.dispatchTable_;
  enabledExtensions_ = std::move(other.enabledExtensions_);
  features_ = other.features_;
  queues_ = std::move(other.queues_);
  physicalDevice_ = std::move(other.physicalDevice_);

  rebindQueues();
  return *this;
}

}  // namespace vkcore