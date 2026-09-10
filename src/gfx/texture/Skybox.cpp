#include "Skybox.h"

#include "TextureLoader.h"

namespace gfx {
Skybox::Skybox(const vkcore::Device& device, vkcore::SampledTexture&& cubemap,
               vkcore::DescriptorSet&& descriptorSet)
    : device_(&device), cubemap_(std::move(cubemap)), descriptorSet_(std::move(descriptorSet)) {

  VkDescriptorImageInfo imageInfo{
      .sampler = cubemap_.sampler().handle(),
      .imageView = cubemap_.imageView().handle(),
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };

  VkWriteDescriptorSet write {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptorSet_.handle(),
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &imageInfo,
  };

  device.dispatchTable().vkUpdateDescriptorSets(device.handle(), 1, &write, 0, nullptr);
}

std::optional<Skybox> Skybox::Load(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
                           vkcore::MemoryAllocator& memoryAllocator,
                           vkcore::DescriptorPool& descriptorPool,
                           const vkcore::DescriptorSetLayout& descriptorSetLayout,
                           const std::array<std::string, 6>& paths, uint32_t mipLevels) {
  if (auto cubemap =
          TextureLoader::LoadCubemap(device, transferCtxt, memoryAllocator, paths, mipLevels)) {
    vkcore::DescriptorSet descriptorSet = descriptorPool.Allocate(descriptorSetLayout);
    return Skybox(device, std::move(cubemap.value()), std::move(descriptorSet));
  }
  return std::nullopt;
}
}  // namespace gfx
