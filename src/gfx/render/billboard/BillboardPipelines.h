#pragma once

#include <array>

#include "../../../vkcore/pipeline/Pipeline.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../../../vkcore/pipeline/RenderPass.h"
#include "../../../vkcore/resource/DescriptorSetLayout.h"
#include "../GameDataBinding.h"
#include "../RenderLayers.h"

namespace gfx {
class BillboardPipelines {
 public:
  BillboardPipelines(const vkcore::Device& device, const vkcore::RenderPass& renderPass,
                     const GameDataBinding& gameDataBinding);

  void Bind(VkCommandBuffer cmd, RenderLayer renderLayer) const {
    assert(renderLayer < RenderLayer::Count);
    pipelines_[static_cast<size_t>(renderLayer)].Bind(cmd);
  }

  [[nodiscard]] const vkcore::DescriptorSetLayout& textureSetLayout() const {
    return textureSetLayout_;
  }

  [[nodiscard]] const vkcore::PipelineLayout& pipelineLayout() const { return pipelineLayout_; }

 private:
  vkcore::DescriptorSetLayout BuildTextureSetLayout(const vkcore::Device& device);
  vkcore::PipelineLayout BuildPipelineLayout(const vkcore::Device& device,
                                             const GameDataBinding& gameDataBinding);
  vkcore::Pipeline BuildPipeline(const vkcore::Device& device, const vkcore::RenderPass& renderPass,
                                 RenderLayer renderLayer);
  std::array<vkcore::Pipeline, static_cast<size_t>(RenderLayer::Count)> BuildPipelines(
      const vkcore::Device& device, const vkcore::RenderPass& renderPass);

 private:
  vkcore::DescriptorSetLayout textureSetLayout_;
  vkcore::PipelineLayout pipelineLayout_;
  std::array<vkcore::Pipeline, static_cast<size_t>(RenderLayer::Count)> pipelines_;
};
}  // namespace gfx
