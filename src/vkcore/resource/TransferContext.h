#pragma once

#include <iostream>
#include <memory>

#include "../commands/CommandBuffer.h"
#include "../commands/CommandPool.h"
#include "BufferAllocator.h"

namespace vkcore {
class TransferContext {
 public:
  struct Allocation {
    void* mapped;
    VkDeviceSize bufferOffset;
    VkBuffer buffer;
  };

  TransferContext(const Device& device, const DeviceQueue& transferQueue,
                  const CommandPool& cmdPool, BufferAllocator& bufferAllocator);
  void Begin();
  void Flush();

  Allocation AllocateStagingBuffer(VkDeviceSize size, VkDeviceSize alignment = 0ull);

  const CommandBuffer& cmd() const { return cmd_; }
  const DeviceQueue& transferQueue() const { return *transferQueue_; }

 private:
  const Device* device_;
  const DeviceQueue* transferQueue_;
  const CommandPool* cmdPool_;
  BufferAllocator* bufferAllocator_;
  std::vector<BufferSlice> reservedSlices_;

  CommandBuffer cmd_;
  bool started_ = false;
};
}  // namespace vkcore
