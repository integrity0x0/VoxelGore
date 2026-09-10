#pragma once

#include "../devices/Device.h"

namespace vkcore {

class DeviceMemory {
 public:
  DeviceMemory(const Device& device, VkDeviceSize size, uint32_t memoryTypeIndex);
  DeviceMemory(const DeviceMemory&) = delete;
  DeviceMemory& operator=(const DeviceMemory&) = delete;
  DeviceMemory(DeviceMemory&& other) = default;

  ~DeviceMemory();

  [[nodiscard]] VkDeviceMemory handle() const { return deviceMemory_.get(); }

  [[nodiscard]] VkDeviceSize size() const { return size_; }

  [[nodiscard]] uint32_t memoryTypeIndex() const { return memoryTypeIndex_; }

  [[nodiscard]] void* map(VkDeviceSize offset) const;

 private:
  const Device* device_;
  UniqueDeviceMemory deviceMemory_;
  VkDeviceSize size_;
  uint32_t memoryTypeIndex_;

  struct MappedDeleter {
    VkDeviceMemory deviceMemory;
    VkDevice device;
    PFN_vkUnmapMemory pfnUnmapMemory;

    void operator()(void* mapped) { pfnUnmapMemory(device, deviceMemory); }
  };

  std::unique_ptr<void, MappedDeleter> mapped_ = nullptr;
};

}  // namespace vkcore