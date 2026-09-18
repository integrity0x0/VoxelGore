#pragma once

#include <array>

#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../GameDataBinding.h"
#include "../shadow/ShadowPass.h"
#include "ChunkMeshBuilder.h"
#include "../../common/shader/ShaderCompiler.h"

namespace gfx {

enum class ShadowLayer : uint8_t { Solid, Cutout, Count };

class ChunkShadowPipelines {
 public:
  ChunkShadowPipelines(const vkcore::Device& device, const ShadowPass& shadowRenderPass,
                       const GameDataBinding& gameDataBinding,
                       const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout,
                       const ShaderCompiler& shaderCompiler);

  void Bind(VkCommandBuffer cmd, ShadowLayer layer) const;

  [[nodiscard]] const vkcore::PipelineLayout& pipelineLayout() const { return pipelineLayout_; }

 private:
  [[nodiscard]] vkcore::PipelineLayout BuildPipelineLayout(
      const vkcore::Device& device, const GameDataBinding& gameDataBinding,
      const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout);

  [[nodiscard]] vkcore::Pipeline BuildPipeline(VkRenderPass shadowRenderPass,
                                               const ShaderCompiler& shaderCompiler,
                                               ShadowLayer layer) const;
 private:
  const vkcore::Device* device_;
  vkcore::PipelineLayout pipelineLayout_;
  std::array<vkcore::Pipeline, static_cast<size_t>(ShadowLayer::Count)> pipelines_;
};

}  // namespace gfx::render::chunk