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

SampledTexture ImportTextureSampled(const Device& device, TransferContext& transferCtxt,
                                    MemoryAllocator& allocator, std::string_view path,
                                    uint32_t mipLevels) {
  VkSamplerCreateInfo samplerCI = MakeSamplerCI(mipLevels);

  return SampledTexture(ImportTexture(device, transferCtxt, allocator, path, mipLevels),
                        Sampler(device, samplerCI));
}

SampledTexture ImportCubemapSampled(const Device& device, TransferContext& transferCtxt,
                                    MemoryAllocator& allocator,
                                    const std::array<std::string, 6>& paths, uint32_t mipLevels) {
  VkSamplerCreateInfo samplerCI = MakeSamplerCI(mipLevels);

  return SampledTexture(ImportCubemap(device, transferCtxt, allocator, paths, mipLevels),
                        Sampler(device, samplerCI));
}

}  // namespace vkcore