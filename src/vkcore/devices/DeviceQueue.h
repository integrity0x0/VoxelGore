#pragma once

#include <vector>

#include "../common/vulkanFunctions.h"

namespace vkcore {

class DeviceQueue {
 public:
  DeviceQueue(const DeviceDispatchTable& dispatchTable, VkQueue queue, uint32_t queueFamilyIndex,
              VkQueueFlags flags)
      : dispatchTable_(&dispatchTable),
        queue_(queue),
        queueFamilyIndex_(queueFamilyIndex),
        flags_(flags) {}

  void rebindDispatchTable(const DeviceDispatchTable& dispatchTable) noexcept {
    dispatchTable_ = &dispatchTable;
  }

  VkQueue handle() const noexcept { return queue_; }

  uint32_t getQueueFamilyIndex() const noexcept { return queueFamilyIndex_; }

  VkQueueFlags flags() const noexcept { return flags_; }

  void waitIdle() const { dispatchTable_->vkQueueWaitIdle(queue_); }

  void submit(VkCommandBuffer commandBuffer, VkFence fence = VK_NULL_HANDLE) const {
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    dispatchTable_->vkQueueSubmit(queue_, 1, &submitInfo, fence);
  }

  void submit(const std::vector<VkCommandBuffer>& commandBuffers,
              const std::vector<VkSemaphore>& waitSemaphores,
              const std::vector<VkPipelineStageFlags>& waitStages,
              const std::vector<VkSemaphore>& signalSemaphores,
              VkFence fence = VK_NULL_HANDLE) const {
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    submitInfo.pCommandBuffers = commandBuffers.data();

    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();

    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    dispatchTable_->vkQueueSubmit(queue_, 1, &submitInfo, fence);
  }

 private:
  const DeviceDispatchTable* dispatchTable_;
  VkQueue queue_;
  uint32_t queueFamilyIndex_;
  VkQueueFlags flags_;
};

}  // namespace vkcore