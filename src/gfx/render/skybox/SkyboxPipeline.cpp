#include "SkyboxPipeline.h"

#include <array>

#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"

namespace gfx {
vkcore::DescriptorSetLayout SkyboxPipeline::BuildDescriptorSetLayout(const vkcore::Device& device) {
  VkDescriptorSetLayoutBinding binding = {};
  binding.binding = 0u;
  binding.descriptorCount = 1u;
  binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  return vkcore::DescriptorSetLayout(device, std::to_array({binding}));
}

vkcore::PipelineLayout SkyboxPipeline::BuildDescriptorPipelineLayout(
    const vkcore::Device& device, const vkcore::DescriptorSetLayout& gameDataLayout) {
  return vkcore::PipelineLayout(device, std::to_array<const vkcore::DescriptorSetLayout*>(
                                            {&gameDataLayout, &descriptorSetLayout_}));
}

vkcore::Pipeline SkyboxPipeline::BuildPipeline(const vkcore::Device& device,
                                               const vkcore::RenderPass& renderPass) {
  return vkcore::GraphicsPipelineCreator(device)
      .AddShaderStage("<SKYBOX_VERTEX_SHADER>", VK_SHADER_STAGE_VERTEX_BIT)
      .AddShaderStage("<SKYBOX_FRAGMENT_SHADER>", VK_SHADER_STAGE_FRAGMENT_BIT)
      .AddVertexBinding(0, sizeof(glm::vec3))
      .AddVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0)
      .setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
      .setCullMode(VK_CULL_MODE_FRONT_BIT)
      .setDepthTest(true, false, VK_COMPARE_OP_LESS_OR_EQUAL)
      .AddColorBlendAttachment(false)
      .Build(pipelineLayout_.handle(), renderPass.handle());
}

SkyboxPipeline::SkyboxPipeline(const vkcore::Device& device, const vkcore::RenderPass& renderPass,
                               const GameDataBinding& gameDataBinding)
    : descriptorSetLayout_(BuildDescriptorSetLayout(device)),
      pipelineLayout_(BuildDescriptorPipelineLayout(device, gameDataBinding.descriptorSetLayout())),
      pipeline_(BuildPipeline(device, renderPass)) {}

}  // namespace gfx