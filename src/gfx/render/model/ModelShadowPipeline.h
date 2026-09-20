#pragma once

#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../../../vkcore/pipeline/RenderPass.h"
#include "../../../vkcore/resource/DescriptorSetLayout.h"
#include "../../common/shader/ShaderCompiler.h"
#include "../GameDataBinding.h"
#include "../shadow/ShadowContext.h"

namespace gfx {
class ModelShadowPipeline {
 public:
  ModelShadowPipeline(const vkcore::Device& device,
                const GameDataBinding& gameDataBinding,
                const vkcore::DescriptorSetLayout& materialSetLayout,
                const ShadowContext& shadowCtxt,
                const ShaderCompiler& shaderCompiler);

  void Bind(VkCommandBuffer cmd) const;

  [[nodiscard]] const vkcore::PipelineLayout& pipelineLayout() const { return pipelineLayout_; }
  [[nodiscard]] const vkcore::Pipeline& pipeline() const { return pipeline_; }

 private:
  [[nodiscard]] vkcore::PipelineLayout BuildPipelineLayout(
      const vkcore::Device& device, const vkcore::DescriptorSetLayout& gameDataSetLayout,
      const vkcore::DescriptorSetLayout& materialSetLayout);

  [[nodiscard]] vkcore::Pipeline BuildPipeline(const vkcore::Device& device,
                                               const vkcore::RenderPass& renderPass,
                                               const ShaderCompiler& shaderCompiler);

 private:
  vkcore::PipelineLayout pipelineLayout_;
  vkcore::Pipeline pipeline_;
};
}  // namespace gfx
