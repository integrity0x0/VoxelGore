#pragma once

#include <array>

#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../GameDataBinding.h"
#include "../RenderLayers.h"
#include "ChunkMeshBuilder.h"

namespace gfx {

class ChunkRenderPipelines {
 public:
  ChunkRenderPipelines(const vkcore::Device& device, VkRenderPass renderPass,
                       const GameDataBinding& gameDataBinding,
                       const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout);

  void Bind(VkCommandBuffer cmd, RenderLayer renderLayer) const;

  const vkcore::PipelineLayout& pipelineLayout() const { return pipelineLayout_; }

 private:
  vkcore::PipelineLayout MakePipelineLayout(
      const vkcore::Device& device, const GameDataBinding& gameDataBinding,
      const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout);

  vkcore::Pipeline MakePipeline(VkRenderPass renderPass, RenderLayer renderLayer) const;

  const vkcore::Device* device_;
  vkcore::PipelineLayout pipelineLayout_;
  std::array<vkcore::Pipeline, static_cast<size_t>(RenderLayer::Count)> pipelines_;
};

}  // namespace gfx