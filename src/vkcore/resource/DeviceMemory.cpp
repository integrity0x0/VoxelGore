#include "DeviceMemory.h"

namespace vkcore {

DeviceMemory::DeviceMemory(const Device& device, VkDeviceSize size, uint32_t memoryTypeIndex)
    : device_(&device), memoryTypeIndex_(memoryTypeIndex), size_(size) {
  VkMemoryAllocateInfo memoryAI = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};

  memoryAI.allocationSize = size;
  memoryAI.memoryTypeIndex = memoryTypeIndex;

  VkDeviceMemory deviceMemoryRaw = VK_NULL_HANDLE;

  SystemError::Check(device_->dispatchTable().vkAllocateMemory(device_->handle(), &memoryAI,
                                                               nullptr, &deviceMemoryRaw),
                     "failed to allocate device memory");

  deviceMemory_ = UniqueDeviceMemory(deviceMemoryRaw,
                                     {device_->handle(), device_->dispatchTable().vkFreeMemory});

  if (device_->getPhysicalDevice()
          .getMemoryProperties()
          .memoryTypes[memoryTypeIndex]
          .propertyFlags &
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
    void* mapped = nullptr;
    SystemError::Check(device_->dispatchTable().vkMapMemory(device_->handle(), deviceMemoryRaw, 0,
                                                            size, 0, &mapped),
                       "failed to map memory");
    MappedDeleter deleter = {deviceMemoryRaw, device.handle(),
                             device.dispatchTable().vkUnmapMemory};

    mapped_ = std::unique_ptr<void, MappedDeleter>(mapped, deleter);
  }
}

DeviceMemory::~DeviceMemory() {}

void* DeviceMemory::map(VkDeviceSize offset) const {
  if (offset >= size_ || !mapped_) {
    return nullptr;
  }

  return static_cast<uint8_t*>(mapped_.get()) + offset;
}

}  // namespace vkcore