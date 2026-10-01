#pragma once

#include "../devices/Device.h"
#include "CommandBuffer.h"

namespace vkcore {

class CommandPool {
 public:
  CommandPool(const Device& device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0)
      : queueFamilyIndex_(queueFamilyIndex), flags_(flags), device_(&device) {
    VkCommandPoolCreateInfo poolCI = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolCI.flags = flags_;
    poolCI.queueFamilyIndex = queueFamilyIndex_;

    VkCommandPool rawPool = VK_NULL_HANDLE;
    SystemError::Check(
        device.GetDispatchTable().vkCreateCommandPool(device.GetHandle(), &poolCI, nullptr, &rawPool),
        "failed to create command pool");

    CommandPoolDeleter deleter{device.GetHandle(), device.GetDispatchTable().vkDestroyCommandPool};
    commandPool_ = UniqueCommandPool(rawPool, deleter);
  }

  VkCommandPool GetHandle() const noexcept { return commandPool_.get(); }
  uint32_t getQueueFamilyIndex() const noexcept { return queueFamilyIndex_; }
  VkCommandPoolCreateFlags flags() const noexcept { return flags_; }

  CommandBuffer Allocate(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const {
    VkCommandBufferAllocateInfo commandBufferAI = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    commandBufferAI.commandPool = commandPool_.get();
    commandBufferAI.level = level;
    commandBufferAI.commandBufferCount = 1;

    VkCommandBuffer buffer = VK_NULL_HANDLE;
    SystemError::Check(device_->GetDispatchTable().vkAllocateCommandBuffers(device_->GetHandle(),
                                                                         &commandBufferAI, &buffer),
                       "failed to allocate command buffer");

    CommandBufferDeleter deleter{device_->GetHandle(), commandPool_.get(),
                                 device_->GetDispatchTable().vkFreeCommandBuffers};
    return CommandBuffer(*device_, UniqueCommandBuffer(buffer, deleter));
  }

  std::vector<CommandBuffer> Allocate(
      uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const {
    VkCommandBufferAllocateInfo commandBufferAI = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    commandBufferAI.commandPool = commandPool_.get();
    commandBufferAI.level = level;
    commandBufferAI.commandBufferCount = count;

    std::vector<VkCommandBuffer> rawBuffers(count);
    SystemError::Check(device_->GetDispatchTable().vkAllocateCommandBuffers(
                           device_->GetHandle(), &commandBufferAI, rawBuffers.data()),
                       "failed to allocate command buffers");

    std::vector<CommandBuffer> buffers;
    buffers.reserve(count);
    CommandBufferDeleter deleter = {device_->GetHandle(), commandPool_.get(),
                                    device_->GetDispatchTable().vkFreeCommandBuffers};
    for (VkCommandBuffer buf : rawBuffers) {
      buffers.push_back(CommandBuffer(*device_, std::move(UniqueCommandBuffer(buf, deleter))));
    }
    return buffers;
  }

  CommandBuffer beginSingleUse() const {
    vkcore::CommandBuffer cmd = Allocate();

    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    SystemError::Check(device_->GetDispatchTable().vkBeginCommandBuffer(cmd.GetHandle(), &beginInfo),
                       "failed to begin command buffer");

    return cmd;
  }

 private:
  UniqueCommandPool commandPool_ = {};
  uint32_t queueFamilyIndex_ = 0;
  VkCommandPoolCreateFlags flags_ = 0;
  const Device* device_;
};

}  // namespace vkcore