#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "../devices/Device.h"
#include "DeviceMemory.h"
#include "MemoryBlock.h"

namespace vkcore {

#define VKCORE_MEMORY_BLOCK_GROWTH(size) ((size)*3 / 2)

#define VKCORE_MEMORY_BLOCK_MIN_SIZE_HOST_VISIBLE (32ull * 1024 * 1024)
#define VKCORE_MEMORY_BLOCK_MIN_SIZE_DEVICE_LOCAL (64ull * 1024 * 1024)

class MemoryAllocator {
 public:
  explicit MemoryAllocator(const Device& device);

  std::optional<MemorySlice> Allocate(VkDeviceSize size, VkDeviceSize alignment,
                                      uint32_t memoryTypeBits, VkMemoryPropertyFlags flags);

 private:
  const Device* device_;
  std::unordered_map<uint32_t, std::vector<std::unique_ptr<MemoryBlock>>> blocksByType_;
};

}  // namespace vkcore