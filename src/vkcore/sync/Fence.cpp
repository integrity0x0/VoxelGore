#include "Fence.h"

namespace vkcore {

Fence::Fence(const Device& device, VkFenceCreateFlags flags) : device_(&device) {
  VkFenceCreateInfo fenceCI = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  fenceCI.flags = flags;

  VkFence rawFence = VK_NULL_HANDLE;
  VkResult result =
      device.dispatchTable().vkCreateFence(device.handle(), &fenceCI, nullptr, &rawFence);
  SystemError::Check(result, "failed to create fence");

  FenceDeleter deleter{device.handle(), device.dispatchTable().vkDestroyFence};
  fence_ = UniqueFence(rawFence, deleter);
}

void Fence::wait(uint64_t timeout) const {
  VkFence handle = fence_.get();
  VkResult result =
      device_->dispatchTable().vkWaitForFences(device_->handle(), 1, &handle, VK_TRUE, timeout);
  SystemError::Check(result, "failed to wait for fence");
}

void Fence::reset() const {
  VkFence handle = fence_.get();
  VkResult result = device_->dispatchTable().vkResetFences(device_->handle(), 1, &handle);
  SystemError::Check(result, "failed to reset fence");
}

bool Fence::isSignaled() const {
  VkResult result = device_->dispatchTable().vkGetFenceStatus(device_->handle(), fence_.get());
  if (result == VK_SUCCESS) return true;
  if (result == VK_NOT_READY) return false;
  SystemError::Check(result, "failed to get fence status");
  return false;
}

}  // namespace vkcore