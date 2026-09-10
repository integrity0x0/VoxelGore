#include "DescriptorSetRegistry.h"

#include <stdexcept>

namespace gfx::ui {

DescriptorSetRegistry::DescriptorSetRegistry(const vkcore::Device& device,
                                             const vkcore::DescriptorSetLayout& layout,
                                             TextureManager& textureManager)
    : device_(&device),
      layout_(&layout),
      textureManager_(&textureManager),
      pool_(device,
            std::array<VkDescriptorPoolSize, 1u>{
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kMaxSets}},
            kMaxSets) {
  sets_.reserve(kMaxSets);
  for (uint32_t i = 0; i < kMaxSets; ++i) {
    sets_.push_back(pool_.Allocate(layout));
  }
}

VkDescriptorSet DescriptorSetRegistry::Get(std::string_view textureKey) {
  if (textureKey.empty()) return VK_NULL_HANDLE;

  const std::string key(textureKey);

  auto it = nameToIndex_.find(key);
  if (it != nameToIndex_.end()) return sets_[it->second].handle();

  const uint32_t index = allocIndex(key);
  const vkcore::SampledTexture* texture = textureManager_->Require(key);
  if (!texture) return VK_NULL_HANDLE;
  writeSet(index, *texture);

  return sets_[index].handle();
}

uint32_t DescriptorSetRegistry::allocIndex(std::string_view textureKey) {
  if (nameToIndex_.size() >= kMaxSets)
    throw std::runtime_error("DescriptorSetRegistry: out of descriptor sets (max 128)");

  const uint32_t index = static_cast<uint32_t>(nameToIndex_.size());
  nameToIndex_.emplace(std::string(textureKey), index);
  return index;
}

void DescriptorSetRegistry::writeSet(uint32_t index, const vkcore::SampledTexture& texture) {
  VkDescriptorImageInfo imageInfo = {};
  imageInfo.sampler = texture.sampler().handle();
  imageInfo.imageView = texture.getTexture().imageView().handle();
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  write.dstSet = sets_[index].handle();
  write.dstBinding = 0;
  write.dstArrayElement = 0;
  write.descriptorCount = 1;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.pImageInfo = &imageInfo;

  device_->dispatchTable().vkUpdateDescriptorSets(device_->handle(), 1, &write, 0, nullptr);
}

}  // namespace gfx::ui