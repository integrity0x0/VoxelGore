#pragma once

#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../../../vkcore/pipeline/RenderPass.h"
#include "../../../vkcore/resource/DescriptorSetLayout.h"

namespace gfx {
class UiRenderPipeline {
 public:

 private:
  [[nodiscard]] vkcore::PipelineLayout BuildPipelineLayout(
      const vkcore::Device& device, const vkcore::DescriptorSetLayout& materialLayout);
  [[nodiscard]] vkcore::Pipeline BuildPipeline(const vkcore::Device& device);
 private:
  vkcore::PipelineLayout pipelineLayout_;
  vkcore::Pipeline pipeline_;
};
}  // namespace gfx
