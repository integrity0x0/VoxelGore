#include "ModelPipeline.h"

#include "../../core/PathPrefixes.h"
#include "../mesh/Model.h"
#include "ModelInstanceData.h"

namespace gfx {
ModelPipeline::ModelPipeline(const vkcore::Device& device, const vkcore::RenderPass& renderPass,
                             const GameDataBinding& gameDataBinding)
    : descriptorSetLayout_(BuildDescriptorSetLayout(device)),
      pipelineLayout_(BuildPipelineLayout(device, gameDataBinding.descriptorSetLayout())),
      pipeline_(BuildPipeline(device, renderPass)) {}

vkcore::DescriptorSetLayout ModelPipeline::BuildDescriptorSetLayout(const vkcore::Device& device) {
  VkDescriptorSetLayoutBinding bindingImage = {};
  bindingImage.binding = 0;
  bindingImage.descriptorCount = 1u;
  bindingImage.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindingImage.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  return vkcore::DescriptorSetLayout(device, std::to_array({bindingImage}));
}

vkcore::PipelineLayout ModelPipeline::BuildPipelineLayout(
    const vkcore::Device& device, const vkcore::DescriptorSetLayout& gameDataBindingLayout) {
  return vkcore::PipelineLayout(device, std::to_array<const vkcore::DescriptorSetLayout*>({
                                            &gameDataBindingLayout,
                                            &descriptorSetLayout_,
                                        }));
}

vkcore::Pipeline ModelPipeline::BuildPipeline(const vkcore::Device& device,
                                              const vkcore::RenderPass& renderPass) {
  constexpr VkDeviceSize kVec3Size = sizeof(glm::vec3);

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
      .AddShaderStage(core::kAssetsPrefix + "shaders/model.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
      .AddShaderStage(core::kAssetsPrefix + "shaders/model.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
      .AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
      .AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
      .setDepthTest(true, true)
      .AddColorBlendAttachment()
      .setCullMode(VK_CULL_MODE_BACK_BIT)
      .Build(pipelineLayout_.handle(), renderPass.handle());
}

void ModelPipeline::Bind(VkCommandBuffer cmd) { pipeline_.Bind(cmd); }
}  // namespace gfx