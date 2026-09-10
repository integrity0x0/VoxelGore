#include "BufferBlock.h"

namespace vkcore {

BufferBlock::BufferBlock(const Device& device, const VkBufferCreateInfo& bufferCI,
                         MemoryAllocator& memoryAllocator, VkMemoryPropertyFlags memProperties)
    : storage_(std::make_shared<BufferBlockStorage>(
          Buffer(device, bufferCI, memoryAllocator, memProperties),
          FreeListAllocator(bufferCI.size))) {}

BufferBlock::BufferBlock(const Device& device, const VkBufferCreateInfo& bufferCI,
                         VkMemoryPropertyFlags memProperties)
    : storage_(std::make_shared<BufferBlockStorage>(Buffer(device, bufferCI, memProperties),
                                                    FreeListAllocator(bufferCI.size))) {}

std::optional<BufferSlice> BufferBlock::Allocate(VkDeviceSize size, VkDeviceSize alignment) {
  auto region = storage_->allocator.reserve(size, alignment);

  if (region) {
    return BufferSlice(storage_, *region);
  } else {
    return std::nullopt;
  }
}
}  // namespace vkcore