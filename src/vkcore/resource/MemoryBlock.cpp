#include "MemoryBlock.h"

namespace vkcore {

MemoryBlock::MemoryBlock(const Device& device, VkDeviceSize size, VkMemoryPropertyFlags flags,
                         uint32_t memoryTypeIndex) {
  storage_ = std::make_shared<MemoryBlockStorage>(DeviceMemory(device, size, memoryTypeIndex),
                                                  FreeListAllocator(size));
}

std::optional<MemorySlice> MemoryBlock::reserve(VkDeviceSize size, VkDeviceSize alignment) {
  if (!storage_) {
    return std::nullopt;
  }

  auto region = storage_->allocator.reserve(size, alignment);

  if (!region) {
    return std::nullopt;
  }

  return MemorySlice(storage_, *region);
}

std::optional<MemorySlice> MemoryBlock::reserveFull() {
  if (!storage_) {
    return std::nullopt;
  }

  auto region = storage_->allocator.reserveFull();

  if (!region) {
    return std::nullopt;
  }

  return MemorySlice(storage_, *region);
}

}  // namespace vkcore