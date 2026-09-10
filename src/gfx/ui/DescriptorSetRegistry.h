#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../../vkcore/devices/Device.h"
#include "../../vkcore/resource/DescriptorPool.h"
#include "../../vkcore/resource/DescriptorSet.h"
#include "../../vkcore/resource/DescriptorSetLayout.h"
#include "../texture/TextureManager.h"

namespace gfx::ui {

class DescriptorSetRegistry {
 public:
  static constexpr uint32_t kMaxSets = 128u;

  DescriptorSetRegistry(const vkcore::Device& device, const vkcore::DescriptorSetLayout& layout,
                        TextureManager& textureManager);

  VkDescriptorSet Get(std::string_view textureKey);

  uint32_t usedCount() const { return static_cast<uint32_t>(nameToIndex_.size()); }
  uint32_t capacity() const { return kMaxSets; }

 private:
  uint32_t allocIndex(std::string_view textureKey);
  void writeSet(uint32_t index, const vkcore::SampledTexture& texture);

  const vkcore::Device* device_;
  const vkcore::DescriptorSetLayout* layout_;
  TextureManager* textureManager_;

  vkcore::DescriptorPool pool_;
  std::vector<vkcore::DescriptorSet> sets_;

  std::unordered_map<std::string, uint32_t> nameToIndex_;
};

}  // namespace gfx::ui