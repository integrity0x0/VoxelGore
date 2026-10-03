#include "Fence.h"

namespace vkcore {

Fence::Fence(const Device& device, VkFenceCreateFlags flags) : device_(&device) {
  VkFenceCreateInfo fenceCI = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  fenceCI.flags = flags;

  VkFence rawFence = VK_NULL_HANDLE;
  VkResult result =
      device.GetDispatchTable().vkCreateFence(device.GetHandle(), &fenceCI, nullptr, &rawFence);
  SystemError::Check(result, "failed to create fence");

  FenceDeleter deleter{device.GetHandle(), device.GetDispatchTable().vkDestroyFence};
  fence_ = UniqueFence(rawFence, deleter);
}

void Fence::wait(uint64_t timeout) const {
  VkFence handle = fence_.get();
  VkResult result =
      device_->GetDispatchTable().vkWaitForFences(device_->GetHandle(), 1, &handle, VK_TRUE, timeout);
  SystemError::Check(result, "failed to wait for fence");
}

void Fence::reset() const {
  VkFence handle = fence_.get();
  VkResult result = device_->GetDispatchTable().vkResetFences(device_->GetHandle(), 1, &handle);
  SystemError::Check(result, "failed to reset fence");
}

bool Fence::isSignaled() const {
  VkResult result = device_->GetDispatchTable().vkGetFenceStatus(device_->GetHandle(), fence_.get());
  if (result == VK_SUCCESS) return true;
  if (result == VK_NOT_READY) return false;
  SystemError::Check(result, "failed to Get fence status");
  return false;
}

}  // namespace vkcore