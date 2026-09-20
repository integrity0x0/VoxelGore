#include "ModelPipeline.h"

#include "../../../core/PathPrefixes.h"
#include "../../common/mesh/Model.h"
#include "ModelInstanceData.h"

namespace gfx {
ModelPipeline::ModelPipeline(const vkcore::Device& device, const vkcore::RenderPass& renderPass,
                             const GameDataBinding& gameDataBinding,
                             const vkcore::DescriptorSetLayout& materialSetLayout,
                             const ShadowContext* shadowCtxt, const ShaderCompiler& shaderCompiler)
    : pipelineLayout_(BuildPipelineLayout(device, gameDataBinding.descriptorSetLayout(), materialSetLayout, shadowCtxt)),
      pipeline_(BuildPipeline(device, renderPass, shaderCompiler)) {}

vkcore::PipelineLayout ModelPipeline::BuildPipelineLayout(
    const vkcore::Device& device, const vkcore::DescriptorSetLayout& gameDataSetLayout,
    const vkcore::DescriptorSetLayout& materialSetLayout, const ShadowContext* shadowCtxt) {
  std::vector<const vkcore::DescriptorSetLayout*> layouts = {
    &gameDataSetLayout, &materialSetLayout
  };

  if (shadowCtxt) layouts.emplace_back(&shadowCtxt->descriptorSetLayout());

  return vkcore::PipelineLayout(device, layouts);
}

vkcore::Pipeline ModelPipeline::BuildPipeline(const vkcore::Device& device,
                                              const vkcore::RenderPass& renderPass,
                                              const ShaderCompiler& shaderCompiler) {
  constexpr VkDeviceSize kVec3Size = sizeof(glm::vec3);

  static const std::string kVertexPath = core::kShadersPrefix + "model.vert";
  static const std::string kFragmentPath = core::kShadersPrefix + "model.frag";
  
  vkcore::ShaderModule vertex =
      CompileShaderModule(shaderCompiler, device, kVertexPath, shaderc_vertex_shader);

  vkcore::ShaderModule fragment =
      CompileShaderModule(shaderCompiler, device, kFragmentPath, shaderc_fragment_shader);


  return vkcore::GraphicsPipelineCreator(device)
      .AddVertexBinding(0, sizeof(Model::Vertex))
      .AddVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, pos))
      .AddVertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv))
      .AddVertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, normal))
      .AddVertexBinding(1, sizeof(ModelInstanceData), VK_VERTEX_INPUT_RATE_INSTANCE)
      .AddVertexAttribute(3, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ModelInstanceData, transform))
      .AddVertexAttribute(4, 1, VK_FORMAT_R32G32B32_SFLOAT,
                          offsetof(ModelInstanceData, transform) + kVec3Size * 1)
      .AddVertexAttribute(5, 1, VK_FORMAT_R32G32B32_SFLOAT,
                          offsetof(ModelInstanceData, transform) + kVec3Size * 2)
      .AddVertexAttribute(6, 1, VK_FORMAT_R32G32B32_SFLOAT,
                          offsetof(ModelInstanceData, transform) + kVec3Size * 3)
      .AddVertexAttribute(7, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(ModelInstanceData, color))
      .AddShaderStage(vertex, VK_SHADER_STAGE_VERTEX_BIT)
      .AddShaderStage(fragment, VK_SHADER_STAGE_FRAGMENT_BIT)
      .AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
      .AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
      .setDepthTest(true, true)
      .AddColorBlendAttachment()
      .setCullMode(VK_CULL_MODE_BACK_BIT)
      .Build(pipelineLayout_.handle(), renderPass.handle());
}

void ModelPipeline::Bind(VkCommandBuffer cmd) const { pipeline_.Bind(cmd); }
}  // namespace gfx