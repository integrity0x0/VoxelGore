#include "MaterialManager.h"

#include <array>

namespace gfx {
vkcore::DescriptorPool MaterialManager::BuildDescriptorPool(const vkcore::Device& device) {
  static constexpr uint32_t kMaxTextures = 32u;

  return vkcore::DescriptorPool(device,
                                std::to_array({VkDescriptorPoolSize{
                                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kMaxTextures}}),
                                32u);
}

MaterialManager::MaterialManager(const vkcore::Device& device,
                                 const vkcore::DescriptorSetLayout& descriptorSetLayout,
                                 TextureManager& textureManager)
    : device_(&device),
      descriptorSetLayout_(&descriptorSetLayout),
      textureManager_(&textureManager),
      descriptorPool_(BuildDescriptorPool(device)) {}

const MaterialManager::Material* MaterialManager::Require(std::string_view key) {
  if (const auto* cached = Find(key)) return cached;

  const auto* texture = textureManager_->Require(key);
  if (!texture) return nullptr;

  auto descriptorSet = descriptorPool_.Allocate(*descriptorSetLayout_);

  VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  write.descriptorCount = 1u;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.dstSet = descriptorSet.handle();

  VkDescriptorImageInfo imageInfo = {};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = texture->imageView().handle();
  imageInfo.sampler = texture->sampler().handle();

  write.pImageInfo = &imageInfo;

  device_->dispatchTable().vkUpdateDescriptorSets(device_->handle(), 1u, &write, 0u, nullptr);

  auto material = std::make_unique<Material>(texture, std::move(descriptorSet));

  auto [it, inserted] = materials_.emplace(std::string(key), std::move(material));

  return it->second.get();
}

const MaterialManager::Material* MaterialManager::Find(std::string_view key) const {
  auto it = materials_.find(key);
  return it != materials_.end() ? it->second.get() : nullptr;
}

}  // namespace gfx