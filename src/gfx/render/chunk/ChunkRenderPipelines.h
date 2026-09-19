#pragma once

#include <array>

#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../GameDataBinding.h"
#include "../RenderLayers.h"
#include "ChunkMeshBuilder.h"
#include "../../common/shader/ShaderCompiler.h"

namespace gfx {

class ChunkRenderPipelines {
 public:
  ChunkRenderPipelines(const vkcore::Device& device, VkRenderPass renderPass,
                       const GameDataBinding& gameDataBinding,
                       const vkcore::DescriptorSetLayout* shadowLayout,
                       const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout,
                       const ShaderCompiler& shaderCompiler);

  void Bind(VkCommandBuffer cmd, RenderLayer renderLayer) const;

  [[nodiscard]] const vkcore::PipelineLayout& pipelineLayout() const { return pipelineLayout_; }

 private:
  [[nodiscard]] vkcore::PipelineLayout BuildPipelineLayout(
      const vkcore::Device& device, const GameDataBinding& gameDataBinding, 
      const vkcore::DescriptorSetLayout* shadowLayout,
      const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout);

  [[nodiscard]] vkcore::Pipeline BuildPipeline(VkRenderPass renderPass,
                                               const ShaderCompiler& shaderCompiler,
                                               RenderLayer renderLayer) const;
 private:
  const vkcore::Device* device_;
  vkcore::PipelineLayout pipelineLayout_;
  std::array<vkcore::Pipeline, static_cast<size_t>(RenderLayer::Count)> pipelines_;
};

}  // namespace gfx