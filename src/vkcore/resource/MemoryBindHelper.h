#pragma once

#include <stdexcept>

#include "../devices/Device.h"
#include "MemoryAllocator.h"
#include "MemoryBlock.h"

namespace vkcore::detail {

template <typename Resource>
static MemorySlice reserveDedicated(Resource& resource, const Device& device,
                                    VkMemoryPropertyFlags flags) {
  const VkMemoryRequirements& req = resource.memoryRequirements();

  std::optional<uint32_t> memoryTypeIndex =
      device.getPhysicalDevice().findMemoryType(flags, req.memoryTypeBits);

  if (!memoryTypeIndex)
    throw SystemError(VK_ERROR_UNKNOWN, "failed to find a suitable memory type index");

  MemoryBlock block(device, req.size, flags, *memoryTypeIndex);
  auto slice = block.reserve(req.size, req.alignment);

  if (!slice) throw std::runtime_error("reserveDedicated: failed to reserve memory");

  return std::move(*slice);
}

template <typename Resource>
static MemorySlice reservePooled(Resource& resource, MemoryAllocator& allocator,
                                 VkMemoryPropertyFlags flags) {
  const VkMemoryRequirements& req = resource.memoryRequirements();
  auto slice = allocator.Allocate(req.size, req.alignment, req.memoryTypeBits, flags);

  if (!slice) throw std::runtime_error("reservePooled: failed to reserve memory");

  return std::move(*slice);
}

}  // namespace vkcore::detail