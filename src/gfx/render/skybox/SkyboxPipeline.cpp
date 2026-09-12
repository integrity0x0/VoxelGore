#include "SkyboxPipeline.h"

#include <array>

#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "SkyboxVertex.h"
#include "../../../core/PathPrefixes.h"

namespace gfx {
vkcore::DescriptorSetLayout SkyboxPipeline::BuildDescriptorSetLayout(const vkcore::Device& device) {
  VkDescriptorSetLayoutBinding binding = {};
  binding.binding = 0;
  binding.descriptorCount = 1u;
  binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
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
      .AddShaderStage(core::kAssetsPrefix + "shaders/skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
      .AddShaderStage(core::kAssetsPrefix + "shaders/skybox.frag.spv",
                      VK_SHADER_STAGE_FRAGMENT_BIT)
      .AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
      .AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
      .AddVertexBinding(0, sizeof(SkyboxVertex))  
      .AddVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0)
      .setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
      .setCullMode(VK_CULL_MODE_BACK_BIT)
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