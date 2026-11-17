#pragma once

#include <array>
#include <span>

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
  const Sampler& GetSampler() const { return sampler_; }
  const Image& GetImage() const { return texture_.getImage(); }
  const ImageView& GetImageView() const { return texture_.GetImageView(); }
  uint32_t GetWidth() const { return texture_.width(); }
  uint32_t GetHeight() const { return texture_.height(); }
  uint32_t mipLevels() const { return texture_.mipLevels(); }

 private:
  Texture texture_;
  Sampler sampler_;
};

SampledTexture MakeTextureSampled(const Device& device, TransferContext& transferCtxt,
                                  MemoryAllocator& allocator, std::span<const std::byte> pixels,
                                  uint32_t width, uint32_t height, uint32_t mipLevels = 1u);

SampledTexture MakeCubemapSampled(const Device& device, TransferContext& transferCtxt,
                                  MemoryAllocator& allocator,
                                  const std::array<std::span<const std::byte>, 6>& faces,
                                  uint32_t size, uint32_t mipLevels = 1u);

}  // namespace vkcore