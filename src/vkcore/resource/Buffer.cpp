#include "Buffer.h"

namespace vkcore {

namespace {
static VkMemoryRequirements memoryRequirements(const Device& device, VkBuffer buffer) {
  VkMemoryRequirements req = {};
  device.dispatchTable().vkGetBufferMemoryRequirements(device.handle(), buffer, &req);
  return req;
}

static UniqueBuffer createBuffer(const Device& device, const VkBufferCreateInfo& bufferCI) {
  VkBuffer bufferRaw = VK_NULL_HANDLE;
  SystemError::Check(
      device.dispatchTable().vkCreateBuffer(device.handle(), &bufferCI, nullptr, &bufferRaw),
      "failed to create vulkan buffer");
  return UniqueBuffer(bufferRaw, {device.handle(), device.dispatchTable().vkDestroyBuffer});
}
}  // namespace

Buffer::Buffer(const Device& device, const VkBufferCreateInfo& bufferCI,
               VkMemoryPropertyFlags flags)
    : device_(&device),
      buffer_(createBuffer(device, bufferCI)),
      usage_(bufferCI.usage),
      size_(bufferCI.size),
      memoryRequirements_(vkcore::memoryRequirements(device, buffer_.get())),
      memorySlice_(detail::reserveDedicated(*this, device, flags)) {
  SystemError::Check(
      device.dispatchTable().vkBindBufferMemory(device.handle(), buffer_.get(),
                                                memorySlice_.memory(), memorySlice_.offset()),
      "failed to bind buffer memory");
}

Buffer::Buffer(const Device& device, const VkBufferCreateInfo& bufferCI, MemoryAllocator& allocator,
               VkMemoryPropertyFlags flags)
    : device_(&device),
      buffer_(createBuffer(device, bufferCI)),
      usage_(bufferCI.usage),
      size_(bufferCI.size),
      memoryRequirements_(vkcore::memoryRequirements(device, buffer_.get())),
      memorySlice_(detail::reservePooled(*this, allocator, flags)) {
  SystemError::Check(
      device.dispatchTable().vkBindBufferMemory(device.handle(), buffer_.get(),
                                                memorySlice_.memory(), memorySlice_.offset()),
      "failed to bind buffer memory");
}

}  // namespace vkcore