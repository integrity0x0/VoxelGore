#pragma once

#include "../../../vkcore/pipeline/Pipeline.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../../../vkcore/pipeline/RenderPass.h"
#include "../../../vkcore/resource/DescriptorSetLayout.h"
#include "../GameDataBinding.h"
#include "../../common/shader/ShaderCompiler.h"

namespace gfx {
class SkyboxPipeline {
 public:
  SkyboxPipeline(const vkcore::Device& device, const vkcore::RenderPass& renderPass,
                 const GameDataBinding& gameDataBinding, const ShaderCompiler& shaderCompiler);

  void Bind(VkCommandBuffer cmd) const {
    pipeline_.Bind(cmd);
  }

  const vkcore::DescriptorSetLayout& descriptorSetLayout() const { return descriptorSetLayout_; }
  const vkcore::PipelineLayout& pipelineLayout() const { return pipelineLayout_; }
 private:
  [[nodiscard]] vkcore::DescriptorSetLayout BuildDescriptorSetLayout(const vkcore::Device& device);
  [[nodiscard]] vkcore::PipelineLayout BuildDescriptorPipelineLayout(
      const vkcore::Device& device, const vkcore::DescriptorSetLayout& gameDataLayout);
  [[nodiscard]] vkcore::Pipeline BuildPipeline(const vkcore::Device& device,
                                               const vkcore::RenderPass& renderPass,
                                               const ShaderCompiler& shaderCompiler);
 private:
  vkcore::DescriptorSetLayout descriptorSetLayout_;
  vkcore::PipelineLayout pipelineLayout_;
  vkcore::Pipeline pipeline_;
};
}  // namespace gfx
