#include "UvBuffer.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

namespace gfx::block {

namespace {
vkcore::Buffer createBuffer(const vkcore::Device& device, vkcore::MemoryAllocator& memoryAllocator,
                            uint32_t maxSurfaces) {
  VkBufferCreateInfo bufferCI = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferCI.size = static_cast<VkDeviceSize>(maxSurfaces) * sizeof(UvRegion);
  bufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  return vkcore::Buffer(device, bufferCI, memoryAllocator,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}
}  // namespace

UvBuffer::UvBuffer(const vkcore::Device& device, vkcore::MemoryAllocator& memoryAllocator,
                   uint32_t maxSurfaces)
    : buffer_(createBuffer(device, memoryAllocator, maxSurfaces)),
      maxSurfaces_(maxSurfaces),
      mapped_(reinterpret_cast<UniformUv*>(buffer_.memorySlice().map())) {}

}  // namespace gfx::block