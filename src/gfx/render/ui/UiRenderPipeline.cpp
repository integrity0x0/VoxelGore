#include "UiRenderPipeline.h"

#include <array>

#include "../../../core/PathPrefixes.h"
#include "UiQuadInstance.h"

namespace gfx {
vkcore::PipelineLayout UiRenderPipeline::BuildPipelineLayout(const vkcore::Device& device, const vkcore::DescriptorSetLayout& materialLayout) {
  VkPushConstantRange range = {};
  range.size = sizeof(PushConstant);
  range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  return vkcore::PipelineLayout(device, std::to_array({&materialLayout}), std::to_array({range}));
} 

vkcore::Pipeline UiRenderPipeline::BuildPipeline(const vkcore::Device& device,
                                                 const vkcore::RenderPass& renderPass,
                                                 const ShaderCompiler& shaderCompiler) {
  vkcore::ShaderModule vert =
      CompileShaderModule(shaderCompiler, device, core::kShadersPrefix + "ui.vert", shaderc_vertex_shader);
  vkcore::ShaderModule frag = CompileShaderModule(
      shaderCompiler, device, core::kShadersPrefix + "ui.frag", shaderc_fragment_shader);

  return vkcore::GraphicsPipelineCreator(device)
    .AddShaderStage(vert, VK_SHADER_STAGE_VERTEX_BIT)
    .AddShaderStage(frag, VK_SHADER_STAGE_FRAGMENT_BIT)
    .AddVertexBinding(0, sizeof(UiQuadInstance), VK_VERTEX_INPUT_RATE_INSTANCE)
    .AddVertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(UiQuadInstance, pos))
    .AddVertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(UiQuadInstance, size))
    .AddVertexAttribute(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(UiQuadInstance, uvRect))
    .AddVertexAttribute(3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(UiQuadInstance, color))
    .AddVertexAttribute(4, 0, VK_FORMAT_R32_SFLOAT, offsetof(UiQuadInstance, radius))
    .SetDepthTest(false, false)
    .AddColorBlendAttachment(true)
    .Build(pipelineLayout_.GetHandle(), renderPass.GetHandle());
}

UiRenderPipeline::UiRenderPipeline(const vkcore::Device& device,
                                   const vkcore::DescriptorSetLayout& materialLayout,
                                   const vkcore::RenderPass& renderPass,
                                   const ShaderCompiler& shaderCompiler)
    : pipelineLayout_(BuildPipelineLayout(device, materialLayout)),
      pipeline_(BuildPipeline(device, renderPass, shaderCompiler)) {}
}  // namespace gfx