#pragma once

#include <array>

#include "../../../vkcore/pipeline/Pipeline.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../../../vkcore/pipeline/RenderPass.h"
#include "../../../vkcore/resource/DescriptorSetLayout.h"
#include "../GameDataBinding.h"
#include "../RenderLayers.h"
#include "../../common/shader/ShaderCompiler.h"

namespace gfx {
class BillboardPipelines {
 public:
  BillboardPipelines(const vkcore::Device& device, const vkcore::RenderPass& renderPass,
                     const GameDataBinding& gameDataBinding, const ShaderCompiler& shaderCompiler);

  void Bind(VkCommandBuffer cmd, RenderLayer renderLayer) const {
    assert(renderLayer < RenderLayer::Count);
    pipelines_[static_cast<size_t>(renderLayer)].Bind(cmd);
  }

  [[nodiscard]] const vkcore::DescriptorSetLayout& textureSetLayout() const {
    return textureSetLayout_;
  }

  [[nodiscard]] const vkcore::PipelineLayout& pipelineLayout() const { return pipelineLayout_; }

 private:
  [[nodiscard]] vkcore::DescriptorSetLayout BuildTextureSetLayout(const vkcore::Device& device);
  [[nodiscard]] vkcore::PipelineLayout BuildPipelineLayout(const vkcore::Device& device,
                                             const GameDataBinding& gameDataBinding);

  [[nodiscard]] vkcore::Pipeline BuildPipeline(const vkcore::Device& device,
                                               const vkcore::RenderPass& renderPass,
                                               const ShaderCompiler& shaderCompiler,
                                               RenderLayer renderLayer);

  [[nodiscard]] std::array<vkcore::Pipeline, static_cast<size_t>(RenderLayer::Count)>
  BuildPipelines(
      const vkcore::Device& device, const vkcore::RenderPass& renderPass, 
      const ShaderCompiler& shaderCompiler);

 private:
  vkcore::DescriptorSetLayout textureSetLayout_;
  vkcore::PipelineLayout pipelineLayout_;
  std::array<vkcore::Pipeline, static_cast<size_t>(RenderLayer::Count)> pipelines_;
};
}  // namespace gfx
