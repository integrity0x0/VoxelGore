#pragma once

#include "../devices/Device.h"

namespace vkcore {

class Fence {
 public:
  explicit Fence(const Device& device, VkFenceCreateFlags flags = 0u);

  VkFence handle() const noexcept { return fence_.get(); }

  void wait(uint64_t timeout = UINT64_MAX) const;
  void reset() const;
  bool isSignaled() const;

  Fence(const Fence&) = delete;
  Fence& operator=(const Fence&) = delete;

  Fence(Fence&&) noexcept = default;
  Fence& operator=(Fence&&) = delete;

 private:
  UniqueFence fence_;
  const Device* device_;
};

}  // namespace vkcore