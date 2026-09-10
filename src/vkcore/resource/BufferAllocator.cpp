#include "BufferAllocator.h"

#include <algorithm>

namespace vkcore {

VkBufferUsageFlags BufferAllocator::normalizeUsage(VkBufferUsageFlags usage) {
  if (usage & (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT)) {
    usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
             VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  }

  return usage;
}

const BufferAllocator::BlockSizeMap BufferAllocator::kDefaultBlockSizes = {
    {
        makeKey(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
        64 * 1024 * 1024ull,
    },
    {
        makeKey(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
        32 * 1024 * 1024ull,
    },
    {
        makeKey(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT),
        32 * 1024 * 1024ull,
    },
};

VkDeviceSize BufferAllocator::defaultBlockSize(BlockKey key) {
  if (auto it = kDefaultBlockSizes.find(key); it != kDefaultBlockSizes.end()) {
    return it->second;
  }

  return kDefaultBlockSize;
}

BufferAllocator::BufferAllocator(const Device& device, MemoryAllocator& memoryAllocator)
    : device_(&device), memoryAllocator_(&memoryAllocator) {}

BufferSlice BufferAllocator::Allocate(VkDeviceSize size, VkDeviceSize alignment,
                                      VkBufferUsageFlags usage,
                                      VkMemoryPropertyFlags memProperties) {
  usage = normalizeUsage(usage);

  const BlockKey key = makeKey(usage, memProperties);
  auto& blocks = blocksMapped_[key];

  for (BufferBlock& block : blocks) {
    if (auto slice = block.Allocate(size, alignment)) {
      return std::move(*slice);
    }
  }

  const VkDeviceSize blockSize = std::max(defaultBlockSize(key), size);

  VkBufferCreateInfo bufferCI = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferCI.usage = usage;
  bufferCI.size = blockSize;

  blocks.emplace_back(*device_, bufferCI, *memoryAllocator_, memProperties);

  return *blocks.back().Allocate(size, alignment);
}

}  // namespace vkcore