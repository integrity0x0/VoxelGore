#pragma once

#include <glm/glm.hpp>

#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../../../vkcore/pipeline/RenderPass.h"
#include "../../../vkcore/resource/DescriptorSetLayout.h"
#include "../../common/shader/ShaderCompiler.h"

namespace gfx {
class UiRenderPipeline {
 public:
  struct PushConstant {
    glm::vec2 ndcScale;
  };

  UiRenderPipeline(const vkcore::Device& device, const vkcore::DescriptorSetLayout& materialLayout,
                   const vkcore::RenderPass& renderPass, const ShaderCompiler& shaderCompiler);

  void Bind(VkCommandBuffer cmd) const {
    pipeline_.Bind(cmd);
  }

  [[nodiscard]] const vkcore::PipelineLayout& GetLayout() const { return pipelineLayout_; }
 private:
  [[nodiscard]] vkcore::PipelineLayout BuildPipelineLayout(
      const vkcore::Device& device, const vkcore::DescriptorSetLayout& materialLayout);
  [[nodiscard]] vkcore::Pipeline BuildPipeline(const vkcore::Device& device,
                                               const vkcore::RenderPass& renderPass,
                                               const ShaderCompiler& shaderCompiler);
 private:
  vkcore::PipelineLayout pipelineLayout_;
  vkcore::Pipeline pipeline_;
};
}  // namespace gfx