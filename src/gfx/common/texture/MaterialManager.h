#pragma once

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../../../util/hashers.h"
#include "../../../vkcore/resource/DescriptorPool.h"
#include "../../../vkcore/resource/DescriptorSet.h"
#include "../../../vkcore/resource/DescriptorSetLayout.h"
#include "TextureManager.h"

namespace gfx {

using MaterialId = uint32_t;

inline constexpr MaterialId kInvalidMaterialId = std::numeric_limits<MaterialId>::max();

struct Material {
  MaterialId id;
  const vkcore::SampledTexture* texture;
  vkcore::DescriptorSet descriptorSet;

  Material(MaterialId id, const vkcore::SampledTexture* texture,
           vkcore::DescriptorSet&& descriptorSet)
      : id(id), texture(texture), descriptorSet(std::move(descriptorSet)) {}
};

class MaterialManager {
 public:
  MaterialManager(const vkcore::Device& device, TextureManager& textureManager);

  [[nodiscard]] MaterialId Require(std::string_view key);
  [[nodiscard]] MaterialId Load(std::string_view path, std::string_view key);
  [[nodiscard]] MaterialId Load(std::string_view path) { return Load(path, path); }
  [[nodiscard]] MaterialId Find(std::string_view key) const;

  [[nodiscard]] const Material& Get(MaterialId id) const;
  [[nodiscard]] const Material* TryGet(MaterialId id) const;

  [[nodiscard]] const vkcore::DescriptorSetLayout& GetDescriptorSetLayout() const {
    return descriptorSetLayout_;
  }

 private:
  [[nodiscard]] MaterialId Register(std::string_view key, const vkcore::SampledTexture& texture);
  [[nodiscard]] vkcore::DescriptorSet AllocateAndWriteDescriptorSet(
      const vkcore::SampledTexture& texture);

  const vkcore::Device& device_;
  TextureManager& textureManager_;
  vkcore::DescriptorSetLayout descriptorSetLayout_;
  vkcore::DescriptorPool descriptorPool_;
  std::vector<std::unique_ptr<Material>> materials_;
  std::unordered_map<std::string, MaterialId, util::StringHash, std::equal_to<>> idByKey_;
};

}  // namespace gfx