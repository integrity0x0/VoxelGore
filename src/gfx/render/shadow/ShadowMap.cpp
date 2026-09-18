#include "ShadowMap.h"

namespace gfx::shadow {
vkcore::SampledTexture ShadowMap::CreateTexture(const vkcore::Device& device, 
                                                vkcore::MemoryAllocator& memoryAllocator,
                                                const VkExtent2D& resolution) {
  VkSamplerCreateInfo samplerCI = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  samplerCI.maxLod = 1.0f;
  samplerCI.addressModeU = samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  samplerCI.compareEnable = VK_TRUE;
  
  VkImageCreateInfo imageCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  imageCI.extent = {.width = resolution.width, .height = resolution.height, .depth = 1};
  imageCI.usage = kTextureUsage;
  imageCI.format = kTextureFormat;
  imageCI.imageType = VK_IMAGE_TYPE_2D;
  imageCI.arrayLayers = 1;

  VkImageViewCreateInfo imageViewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  imageViewCI.format = kTextureFormat;
  imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  imageViewCI.subresourceRange.layerCount = 1;
  imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;

  return vkcore::SampledTexture(device, memoryAllocator, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageCI, imageViewCI, samplerCI);
}

ShadowMap::ShadowMap(const vkcore::Device& device, vkcore::MemoryAllocator& memoryAllocator,
                     const ShadowPass& shadowPass, const VkExtent2D& resolution)
    : texture_(CreateTexture(device, memoryAllocator, resolution)), framebuffer_(shadowPass.MakeFramebuffer(&texture_.imageView(), resolution)){

}
}  // namespace gfx