#include "MaterialManager.h"

#include <array>
#include <cassert>

namespace gfx {
namespace {

constexpr uint32_t kMaxMaterials = 1024;

vkcore::DescriptorPool BuildDescriptorPool(const vkcore::Device& device) {
  return vkcore::DescriptorPool(device,
                                std::to_array({VkDescriptorPoolSize{
                                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kMaxMaterials}}),
                                kMaxMaterials);
}

vkcore::DescriptorSetLayout BuildDescriptorSetLayout(const vkcore::Device& device) {
  VkDescriptorSetLayoutBinding bindingImage = {};
  bindingImage.binding = 0;
  bindingImage.descriptorCount = 1u;
  bindingImage.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindingImage.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  return vkcore::DescriptorSetLayout(device, std::to_array({bindingImage}));
}

}  // namespace

MaterialManager::MaterialManager(const vkcore::Device& device, TextureManager& textureManager)
    : device_(device),
      textureManager_(textureManager),
      descriptorSetLayout_(BuildDescriptorSetLayout(device)),
      descriptorPool_(BuildDescriptorPool(device)) {}

MaterialId MaterialManager::Find(std::string_view key) const {
  const auto it = idByKey_.find(key);
  return it != idByKey_.end() ? it->second : kInvalidMaterialId;
}

MaterialId MaterialManager::Require(std::string_view key) {
  if (const MaterialId id = Find(key); id != kInvalidMaterialId) return id;

  const auto* texture = textureManager_.Require(key);
  return texture ? Register(key, *texture) : kInvalidMaterialId;
}

MaterialId MaterialManager::Load(std::string_view path, std::string_view key) {
  if (const MaterialId id = Find(key); id != kInvalidMaterialId) return id;

  const auto* texture = textureManager_.Load(path, key);
  return texture ? Register(key, *texture) : kInvalidMaterialId;
}

MaterialId MaterialManager::Register(std::string_view key, const vkcore::SampledTexture& texture) {
  const auto id = static_cast<MaterialId>(materials_.size());
  assert(id != kInvalidMaterialId);

  vkcore::DescriptorSet descriptorSet = AllocateAndWriteDescriptorSet(texture);
  materials_.push_back(std::make_unique<Material>(id, &texture, std::move(descriptorSet)));
  idByKey_.emplace(std::string(key), id);
  return id;
}

const Material& MaterialManager::Get(MaterialId id) const {
  assert(id < materials_.size());
  return *materials_[id];
}

const Material* MaterialManager::TryGet(MaterialId id) const {
  return id < materials_.size() ? materials_[id].get() : nullptr;
}

vkcore::DescriptorSet MaterialManager::AllocateAndWriteDescriptorSet(
    const vkcore::SampledTexture& texture) {
  auto descriptorSet = descriptorPool_.Allocate(descriptorSetLayout_);

  VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  write.descriptorCount = 1u;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.dstSet = descriptorSet.GetHandle();

  VkDescriptorImageInfo imageInfo = {};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = texture.GetImageView().GetHandle();
  imageInfo.sampler = texture.GetSampler().GetHandle();
  write.pImageInfo = &imageInfo;

  device_.GetDispatchTable().vkUpdateDescriptorSets(device_.GetHandle(), 1u, &write, 0, nullptr);
  return descriptorSet;
}

}  // namespace gfx