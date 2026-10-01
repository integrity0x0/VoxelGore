#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <limits>

#include "../../../util/hashers.h"
#include "../../../vkcore/resource/DescriptorPool.h"
#include "../../../vkcore/resource/DescriptorSetLayout.h"
#include "TextureManager.h"

namespace gfx {

using MaterialId = uint32_t;

static constexpr MaterialId kInvalidMaterialId = std::numeric_limits<MaterialId>::max();

struct Material {
  const vkcore::SampledTexture* texture;
  vkcore::DescriptorSet descriptorSet;

  Material(const vkcore::SampledTexture* texture, vkcore::DescriptorSet&& descriptorSet)
      : texture(texture), descriptorSet(std::move(descriptorSet)) {}
};

class MaterialManager {
 public:

  MaterialManager(const vkcore::Device& device, TextureManager& textureManager);

  [[nodiscard]] const Material* Require(std::string_view key);
  [[nodiscard]] const Material* Find(std::string_view key) const;
  [[nodiscard]] const vkcore::DescriptorSetLayout& GetDescriptorSetLayout() const { return descriptorSetLayout_; }
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