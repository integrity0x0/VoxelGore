#include "MemoryAllocator.h"

#include <algorithm>
#include <stdexcept>

namespace vkcore {

MemoryAllocator::MemoryAllocator(const Device& device) : device_(&device) {}

std::optional<MemorySlice> MemoryAllocator::Allocate(VkDeviceSize size, VkDeviceSize alignment,
                                                     uint32_t memoryTypeBits,
                                                     VkMemoryPropertyFlags flags) {
  std::optional<uint32_t> memoryTypeIndex =
      device_->getPhysicalDevice().findMemoryType(flags, memoryTypeBits);
  if (!memoryTypeIndex.has_value()) throw std::runtime_error("failed to find suitable memory type");
  std::vector<std::unique_ptr<MemoryBlock>>& blocks = blocksByType_[*memoryTypeIndex];

  for (auto& block : blocks) {
    if (auto slice = block->reserve(size, alignment)) {
      return slice;
    }
  }

  VkDeviceSize minBlockSize = (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                                  ? VKCORE_MEMORY_BLOCK_MIN_SIZE_HOST_VISIBLE
                                  : VKCORE_MEMORY_BLOCK_MIN_SIZE_DEVICE_LOCAL;

  VkDeviceSize blockSize = std::max(VKCORE_MEMORY_BLOCK_GROWTH(size), minBlockSize);

  auto newBlock = std::make_unique<MemoryBlock>(*device_, blockSize, flags, *memoryTypeIndex);

  auto slice = newBlock->reserve(size, alignment);

  blocks.push_back(std::move(newBlock));

  return slice;
}

}  // namespace vkcore