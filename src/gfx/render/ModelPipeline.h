#pragma once

#include "../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "../../vkcore/pipeline/PipelineLayout.h"
#include "../../vkcore/pipeline/RenderPass.h"
#include "../../vkcore/resource/DescriptorSetLayout.h"
#include "GameDataBinding.h"
#include "../common/shader/ShaderCompiler.h"

namespace gfx {
class ModelPipeline {
 public:
  ModelPipeline(const vkcore::Device& device, const vkcore::RenderPass& renderPass,
                const GameDataBinding& gameDataBinding, const ShaderCompiler& shaderCompiler);

  void Bind(VkCommandBuffer cmd);

  [[nodiscard]] const vkcore::DescriptorSetLayout& descriptorSetLayout() const {
    return descriptorSetLayout_;
  }
  [[nodiscard]] const vkcore::PipelineLayout& pipelineLayout() const { return pipelineLayout_; }
  [[nodiscard]] const vkcore::Pipeline& pipeline() const { return pipeline_; }

 private:
  [[nodiscard]] vkcore::DescriptorSetLayout BuildDescriptorSetLayout(const vkcore::Device& device);
  [[nodiscard]] vkcore::PipelineLayout BuildPipelineLayout(
      const vkcore::Device& device, const vkcore::DescriptorSetLayout& gameDataBindingLayout);

  [[nodiscard]] vkcore::Pipeline BuildPipeline(const vkcore::Device& device,
                                               const vkcore::RenderPass& renderPass,
                                               const ShaderCompiler& shaderCompiler);

 private:
  vkcore::DescriptorSetLayout descriptorSetLayout_;
  vkcore::PipelineLayout pipelineLayout_;
  vkcore::Pipeline pipeline_;
};
}  // namespace gfx
