#include "SampledTexture.h"

namespace vkcore {

namespace {

VkSamplerCreateInfo MakeSamplerCI(uint32_t mipLevels) {
  VkSamplerCreateInfo samplerCI = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  samplerCI.maxLod = static_cast<float>(mipLevels);
  return samplerCI;
}

Sampler CreateSampler(const Device& device, const VkImageCreateInfo& imageCI,
                      VkSamplerCreateInfo samplerCI) {
  samplerCI.maxLod = static_cast<float>(imageCI.mipLevels);
  return Sampler(device, samplerCI);
}

}  // namespace

SampledTexture::SampledTexture(const Device& device, MemoryAllocator& memoryAllocator,
                               VkMemoryPropertyFlags memoryProperties,
                               const VkImageCreateInfo& imageCI,
                               const VkImageViewCreateInfo& viewCI,
                               const VkSamplerCreateInfo& samplerCI)
    : texture_(device, memoryAllocator, memoryProperties, imageCI, viewCI),
      sampler_(CreateSampler(device, imageCI, samplerCI)) {}

SampledTexture::SampledTexture(const Device& device, VkMemoryPropertyFlags memoryProperties,
                               const VkImageCreateInfo& imageCI,
                               const VkImageViewCreateInfo& viewCI,
                               const VkSamplerCreateInfo& samplerCI)
    : texture_(device, memoryProperties, imageCI, viewCI),
      sampler_(CreateSampler(device, imageCI, samplerCI)) {}

SampledTexture MakeTextureSampled(const Device& device, TransferContext& transferCtxt,
                                  MemoryAllocator& allocator, std::span<const std::byte> pixels,
                                  uint32_t width, uint32_t height, uint32_t mipLevels) {
  return SampledTexture(
      MakeTexture(device, transferCtxt, allocator, pixels, width, height, mipLevels),
      Sampler(device, MakeSamplerCI(mipLevels)));
}

SampledTexture MakeCubemapSampled(const Device& device, TransferContext& transferCtxt,
                                  MemoryAllocator& allocator,
                                  const std::array<std::span<const std::byte>, 6>& faces,
                                  uint32_t size, uint32_t mipLevels) {
  return SampledTexture(MakeCubemap(device, transferCtxt, allocator, faces, size, mipLevels),
                        Sampler(device, MakeSamplerCI(mipLevels)));
}

}  // namespace vkcore