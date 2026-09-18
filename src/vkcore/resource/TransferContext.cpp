#include "TransferContext.h"

namespace vkcore {
TransferContext::TransferContext(const Device& device, const DeviceQueue& transferQueue,
                                 const CommandPool& cmdPool, BufferAllocator& bufferAllocator)
    : device_(&device),
      transferQueue_(&transferQueue),
      cmdPool_(&cmdPool),
      cmd_(cmdPool.Allocate()),
      bufferAllocator_(&bufferAllocator) {}

void TransferContext::Begin() {
  if (started_) {
    Flush();
  }
  cmd_.begin();

  started_ = true;
}

void TransferContext::Flush() {
  cmd_.end();
  transferQueue_->Submit(cmd_.handle());

  transferQueue_->WaitIdle();

  reservedSlices_.clear();

  started_ = false;
}

TransferContext::Allocation TransferContext::AllocateStagingBuffer(VkDeviceSize size,
                                                                   VkDeviceSize alignment) {
  BufferSlice slice = bufferAllocator_->Allocate(
      size, alignment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  TransferContext::Allocation allocation;
  allocation.mapped = slice.map();
  allocation.bufferOffset = slice.offset();
  allocation.buffer = slice.handle();

  reservedSlices_.emplace_back(std::move(slice));

  return allocation;
}

}  // namespace vkcore