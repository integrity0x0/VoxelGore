#pragma once

#include "../../../vkcore/resource/SampledTexture.h"
#include "ShadowPass.h"

namespace gfx {
class ShadowMap {
 public:
  ShadowMap(const vkcore::Device& device, vkcore::MemoryAllocator& memoryAllocator,
            const ShadowPass& shadowPass, const VkExtent2D& resolution = {4096, 4096});

  [[nodiscard]] const vkcore::Framebuffer& framebuffer() const { return framebuffer_; }
  [[nodiscard]] const vkcore::SampledTexture& texture() const { return texture_; }
 private:
  [[nodiscard]] vkcore::SampledTexture CreateTexture(const vkcore::Device& device,
                                                     vkcore::MemoryAllocator& memoryAllocator,
                                                     const VkExtent2D& resolution);
 private:
  static constexpr VkImageUsageFlags kTextureUsage =
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  vkcore::SampledTexture texture_;
  vkcore::Framebuffer framebuffer_;
};
}  // namespace gfx
