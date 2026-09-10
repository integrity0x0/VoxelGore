#include "CommandBuffer.h"

#include <stdexcept>

namespace vkcore {

void CommandBuffer::begin(VkCommandBufferUsageFlags flags) {
  VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  beginInfo.flags = flags;

  SystemError::check(device_->dispatchTable().vkBeginCommandBuffer(handle(), &beginInfo),
                     "failed to begin command buffer");
}

void CommandBuffer::end() {
  SystemError::check(device_->dispatchTable().vkEndCommandBuffer(handle()),
                     "failed to end command buffer");
}

}  // namespace vkcore