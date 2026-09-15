#pragma once

#include <array>

#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../GameDataBinding.h"
#include "../shadow/ShadowPass.h"
#include "ChunkMeshBuilder.h"

namespace gfx::render::chunk {

enum class ShadowLayer : uint8_t { Solid, Cutout, Count };

class ShadowPipelines {
 public:
  ShadowPipelines(const vkcore::Device& device, const shadow::ShadowPass& shadowRenderPass,
                  const GameDataBinding& gameDataBinding,
                  const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout);

  void Bind(VkCommandBuffer cmd, ShadowLayer layer) const;

  const vkcore::PipelineLayout& pipelineLayout() const { return pipelineLayout_; }

 private:
  vkcore::PipelineLayout BuildPipelineLayout(
      const vkcore::Device& device, const GameDataBinding& gameDataBinding,
      const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout);

  vkcore::Pipeline BuildPipeline(VkRenderPass shadowRenderPass, ShadowLayer layer) const;

  const vkcore::Device* device_;
  vkcore::PipelineLayout pipelineLayout_;
  std::array<vkcore::Pipeline, static_cast<size_t>(ShadowLayer::Count)> pipelines_;
};

}  // namespace gfx::render::chunk