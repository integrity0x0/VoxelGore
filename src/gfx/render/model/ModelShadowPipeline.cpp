#include "../../../core/PathPrefixes.h"
#include "../../common/mesh/Model.h"
#include "ModelInstanceData.h"
#include "ModelShadowPipeline.h"

namespace gfx {
ModelShadowPipeline::ModelShadowPipeline(const vkcore::Device& device,
                                         const GameDataBinding& gameDataBinding,
                                         const vkcore::DescriptorSetLayout& materialSetLayout,
                                         const ShadowContext& shadowCtxt,
                                         const ShaderCompiler& shaderCompiler)
    : pipelineLayout_(BuildPipelineLayout(device, gameDataBinding.descriptorSetLayout(),
                                          materialSetLayout)),
      pipeline_(BuildPipeline(device, shadowCtxt.pass(), shaderCompiler)) {}

vkcore::PipelineLayout ModelShadowPipeline::BuildPipelineLayout(
    const vkcore::Device& device, const vkcore::DescriptorSetLayout& gameDataSetLayout,
    const vkcore::DescriptorSetLayout& materialSetLayout) {
  return vkcore::PipelineLayout(device, std::to_array({&gameDataSetLayout, &materialSetLayout}));
}

vkcore::Pipeline ModelShadowPipeline::BuildPipeline(const vkcore::Device& device,
                                                    const vkcore::RenderPass& renderPass,
                                                    const ShaderCompiler& shaderCompiler) {
  constexpr VkDeviceSize kVec3Size = sizeof(glm::vec3);

  static const std::string kVertexPath = core::kShadersPrefix + "model_shadow.vert";
  static const std::string kFragmentPath = core::kShadersPrefix + "model_shadow.frag";

  vkcore::ShaderModule vertex =
      CompileShaderModule(shaderCompiler, device, kVertexPath, shaderc_vertex_shader);

  vkcore::ShaderModule fragment =
      CompileShaderModule(shaderCompiler, device, kFragmentPath, shaderc_fragment_shader);

  return vkcore::GraphicsPipelineCreator(device)
      .AddVertexBinding(0, sizeof(Model::Vertex))
      .AddVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, pos))
   // .AddVertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv))
   // .AddVertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, normal))
      .AddVertexBinding(1, sizeof(ModelInstanceData), VK_VERTEX_INPUT_RATE_INSTANCE)
      .AddVertexAttribute(3, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ModelInstanceData, transform))
      .AddVertexAttribute(4, 1, VK_FORMAT_R32G32B32_SFLOAT,
                          offsetof(ModelInstanceData, transform) + kVec3Size * 1)
      .AddVertexAttribute(5, 1, VK_FORMAT_R32G32B32_SFLOAT,
                          offsetof(ModelInstanceData, transform) + kVec3Size * 2)
      .AddVertexAttribute(6, 1, VK_FORMAT_R32G32B32_SFLOAT,
                          offsetof(ModelInstanceData, transform) + kVec3Size * 3)
   // .AddVertexAttribute(7, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(ModelInstanceData, color))
      .AddShaderStage(vertex, VK_SHADER_STAGE_VERTEX_BIT)
      .AddShaderStage(fragment, VK_SHADER_STAGE_FRAGMENT_BIT)
      .AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
      .AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
      .setDepthTest(true, true)
      .setCullMode(VK_CULL_MODE_BACK_BIT)
      .Build(pipelineLayout_.handle(), renderPass.handle());
}

void ModelShadowPipeline::Bind(VkCommandBuffer cmd) const { pipeline_.Bind(cmd); }
}  // namespace gfx