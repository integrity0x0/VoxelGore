#pragma once

#include "Sampler.h"
#include "Texture.h"

namespace vkcore {
class SampledTexture {
 public:
  SampledTexture(Texture&& texture, Sampler&& sampler)
      : texture_(std::move(texture)), sampler_(std::move(sampler)) {}
  SampledTexture(const Device& device, MemoryAllocator& memoryAllocator,
                 VkMemoryPropertyFlags memoryProperties, const VkImageCreateInfo& imageCI,
                 const VkImageViewCreateInfo& viewCI, const VkSamplerCreateInfo& samplerCI);
  SampledTexture(const Device& device, VkMemoryPropertyFlags memoryProperties,
                 const VkImageCreateInfo& imageCI, const VkImageViewCreateInfo& viewCI,
                 const VkSamplerCreateInfo& samplerCI);

  const Texture& getTexture() const { return texture_; }
  const Sampler& sampler() const { return sampler_; }
  const Image& image() const { return texture_.getImage(); }
  const ImageView& imageView() const { return texture_.imageView(); }
  uint32_t width() const { return texture_.width(); }
  uint32_t height() const { return texture_.height(); }
  uint32_t mipLevels() const { return texture_.mipLevels(); }

 private:
  Texture texture_;
  Sampler sampler_;
};

extern SampledTexture ImportTextureSampled(const Device& device, TransferContext& transferCtxt,
                                           MemoryAllocator& allocator, std::string_view path,
                                           uint32_t mipLevels = 1u);

extern SampledTexture ImportCubemapSampled(const Device& device, TransferContext& transferCtxt,
                                           MemoryAllocator& allocator,
                                           const std::array<std::string, 6>& paths,
                                           uint32_t mipLevels = 1u);
}  // namespace vkcore