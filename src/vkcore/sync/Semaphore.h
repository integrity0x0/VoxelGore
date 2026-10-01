#pragma once

#include "../devices/Device.h"

namespace vkcore {

class Semaphore {
 public:
  explicit Semaphore(const Device& device, VkSemaphoreCreateFlags flags = 0);

  VkSemaphore GetHandle() const noexcept { return semaphore_.get(); }

 private:
  UniqueSemaphore semaphore_;
  const Device* device_;
};

}  // namespace vkcore