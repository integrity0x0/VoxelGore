#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "BufferBlock.h"

namespace vkcore {

class BufferAllocator {
 public:
  BufferAllocator(const Device& device, MemoryAllocator& memoryAllocator);

  BufferSlice Allocate(VkDeviceSize size, VkDeviceSize alignment, VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags memProperties);

  BufferSlice Allocate(VkDeviceSize size, VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags memProperties) {
    return Allocate(size, 1ull, usage, memProperties);
  }

 private:
  using BlockKey = uint64_t;
  using BlockSizeMap = std::unordered_map<BlockKey, VkDeviceSize>;

  static constexpr VkDeviceSize kDefaultBlockSize = 16 * 1024 * 1024;

  static constexpr BlockKey makeKey(VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties) {
    return (static_cast<uint64_t>(usage) << 32) | static_cast<uint32_t>(memProperties);
  }

  static VkBufferUsageFlags normalizeUsage(VkBufferUsageFlags usage);
  static VkDeviceSize defaultBlockSize(BlockKey key);

  static const BlockSizeMap kDefaultBlockSizes;

  const Device* device_;
  MemoryAllocator* memoryAllocator_;

  std::unordered_map<BlockKey, std::vector<BufferBlock>> blocksMapped_;
};

}  // namespace vkcore