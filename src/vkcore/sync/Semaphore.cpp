#include "Semaphore.h"

namespace vkcore {

Semaphore::Semaphore(const Device& device, VkSemaphoreCreateFlags flags) : device_(&device) {
  VkSemaphoreCreateInfo semaphoreCI = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  semaphoreCI.flags = flags;

  VkSemaphore rawSemaphore = VK_NULL_HANDLE;
  VkResult result = device.dispatchTable().vkCreateSemaphore(device.handle(), &semaphoreCI, nullptr,
                                                             &rawSemaphore);
  SystemError::Check(result, "failed to create semaphore");

  SemaphoreDeleter deleter{device.handle(), device.dispatchTable().vkDestroySemaphore};
  semaphore_ = UniqueSemaphore(rawSemaphore, deleter);
}

}  // namespace vkcore