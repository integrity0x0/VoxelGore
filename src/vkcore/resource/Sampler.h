#pragma once

#include "../devices/Device.h"

namespace vkcore {
class Sampler {
 public:
  Sampler(const Device& device, const VkSamplerCreateInfo& samplerCI) {
    VkSampler samplerRaw = VK_NULL_HANDLE;
    SystemError::Check(
        device.GetDispatchTable().vkCreateSampler(device.GetHandle(), &samplerCI, nullptr, &samplerRaw),
        "failed to create sampler");

    sampler = UniqueSampler(samplerRaw, {device.GetHandle(), device.GetDispatchTable().vkDestroySampler});
  }

  VkSampler GetHandle() const { return sampler.get(); }

 private:
  UniqueSampler sampler;
};
}  // namespace vkcore