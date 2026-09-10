#pragma once

#include "../devices/Device.h"

namespace vkcore {
class Sampler {
 public:
  Sampler(const Device& device, const VkSamplerCreateInfo& samplerCI) {
    VkSampler samplerRaw = VK_NULL_HANDLE;
    SystemError::check(
        device.dispatchTable().vkCreateSampler(device.handle(), &samplerCI, nullptr, &samplerRaw),
        "failed to create sampler");

    sampler = UniqueSampler(samplerRaw, {device.handle(), device.dispatchTable().vkDestroySampler});
  }

  VkSampler handle() const { return sampler.get(); }

 private:
  UniqueSampler sampler;
};
}  // namespace vkcore