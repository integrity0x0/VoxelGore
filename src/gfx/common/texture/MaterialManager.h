#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../../../util/hashers.h"
#include "../../../vkcore/resource/DescriptorPool.h"
#include "../../../vkcore/resource/DescriptorSetLayout.h"
#include "TextureManager.h"

namespace gfx {

class MaterialManager {
 public:
  struct Material {
    const vkcore::SampledTexture* texture;
    vkcore::DescriptorSet descriptorSet;

    Material(const vkcore::SampledTexture* texture, vkcore::DescriptorSet&& descriptorSet)
        : texture(texture), descriptorSet(std::move(descriptorSet)) {}
  };

  MaterialManager(const vkcore::Device& device, TextureManager& textureManager);

  [[nodiscard]] const Material* Require(std::string_view key);
  [[nodiscard]] const Material* Find(std::string_view key) const;
  [[nodiscard]] const vkcore::DescriptorSetLayout& descriptorSetLayout() const { return descriptorSetLayout_; }
 private:
  [[nodiscard]] vkcore::DescriptorPool BuildDescriptorPool(const vkcore::Device& device);
  [[nodiscard]] vkcore::DescriptorSetLayout BuildDescriptorSetLayout(const vkcore::Device& device);
 private:
  const vkcore::Device* device_;
  TextureManager* textureManager_;
  vkcore::DescriptorSetLayout descriptorSetLayout_;
  vkcore::DescriptorPool descriptorPool_;

  std::unordered_map<std::string, std::unique_ptr<Material>, util::StringHash, std::equal_to<>>
      materials_;
};

}  // namespace gfx