#include "memoryUtils.h"

#include <cassert>
#include <cstring>

#include "Buffer.h"
#include "MemoryBlock.h"

namespace vkcore {

void LoadDataToBuffer(const Device& device, TransferContext& transferCtxt,
                      std::span<const std::byte> data, const Buffer& dstBuffer,
                      VkDeviceSize offset) {
  assert(data.size() > 0ull);
  assert(offset + data.size() <= dstBuffer.size());
  assert(dstBuffer.usage() & VK_BUFFER_USAGE_TRANSFER_DST_BIT);

  if (data.size() == 0) {
    return;
  }

  auto allocation = transferCtxt.AllocateStagingBuffer(data.size());

  std::memcpy(allocation.mapped, data.data(), data.size());

  VkBufferCopy copyRegion = {allocation.bufferOffset, offset, data.size()};

  device.dispatchTable().vkCmdCopyBuffer(transferCtxt.cmd().handle(), allocation.buffer,
                                         dstBuffer.handle(), 1u, &copyRegion);
}

}  // namespace vkcore