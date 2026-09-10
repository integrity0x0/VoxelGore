#pragma once

#include "../devices/DeviceCreator.h"
#include "MemoryAllocator.h"
#include "MemoryBindHelper.h"
#include "MemoryBlock.h"

namespace vkcore {

class Buffer {
 public:
  Buffer(const Device& device, const VkBufferCreateInfo& bufferCI, VkMemoryPropertyFlags flags);

  Buffer(const Device& device, const VkBufferCreateInfo& bufferCI, MemoryAllocator& allocator,
         VkMemoryPropertyFlags flags);

  // Rule of Five
  ~Buffer() = default;

  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  Buffer(Buffer&&) noexcept = default;
  Buffer& operator=(Buffer&&) noexcept = default;

  [[nodiscard]] VkBuffer handle() const { return buffer_.get(); }
  [[nodiscard]] VkBufferUsageFlags usage() const { return usage_; }
  [[nodiscard]] VkDeviceSize size() const { return size_; }
  [[nodiscard]] const VkMemoryRequirements& memoryRequirements() const {
    return memoryRequirements_;
  }

  [[nodiscard]] const MemorySlice& memorySlice() const { return memorySlice_; }

  void BindVertex(VkCommandBuffer cmd, VkDeviceSize offset = 0ull,
                  uint32_t firstBinding = 0u) const {
    VkDeviceSize offsets[] = {offset};
    VkBuffer buffers[] = {buffer_.get()};
    device_->dispatchTable().vkCmdBindVertexBuffers(cmd, firstBinding, 1u, buffers, offsets);
  }

  void BindIndex(VkCommandBuffer cmd, VkDeviceSize offset = 0ull,
                 VkIndexType indexType = VK_INDEX_TYPE_UINT32) const {
    device_->dispatchTable().vkCmdBindIndexBuffer(cmd, buffer_.get(), offset, indexType);
  }

 private:
  const Device* device_;
  UniqueBuffer buffer_;
  VkBufferUsageFlags usage_;
  VkDeviceSize size_;
  VkMemoryRequirements memoryRequirements_;
  MemorySlice memorySlice_;
};

}  // namespace vkcore