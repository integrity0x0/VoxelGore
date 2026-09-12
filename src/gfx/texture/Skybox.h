#pragma once

#include <optional>

#include "../../vkcore/resource/DescriptorPool.h"
#include "../../vkcore/resource/DescriptorSetLayout.h"
#include "../../vkcore/resource/SampledTexture.h"

namespace gfx {
class Skybox {
 public:
  Skybox(const vkcore::Device& device, vkcore::SampledTexture&& cubemap, vkcore::DescriptorSet&& descriptorSet);

  static std::optional<Skybox> Load(const vkcore::Device& device,
                                    vkcore::TransferContext& transferCtxt,
                                    vkcore::MemoryAllocator& memoryAllocator,
                                    const vkcore::DescriptorPool& descriptorPool,
                                    const vkcore::DescriptorSetLayout& descriptorSetLayout,
                                    const std::array<std::string, 6>& paths,
                                    uint32_t mipLevels = 1);
  void Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const;
 private:
  static constexpr uint32_t kCubemapBinding = 1u;
  const vkcore::Device* device_;
  vkcore::SampledTexture cubemap_;
  vkcore::DescriptorSet descriptorSet_;
};
}  // namespace gfx
