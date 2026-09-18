#include "BillboardPipelines.h"

#include <array>

#include "../../../core/PathPrefixes.h"
#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "BillboardInstance.h"

namespace gfx {

vkcore::DescriptorSetLayout BillboardPipelines::BuildTextureSetLayout(
    const vkcore::Device& device) {
  VkDescriptorSetLayoutBinding binding = {};
  binding.descriptorCount = 1u;
  binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  return vkcore::DescriptorSetLayout(device, std::to_array<>({binding}));
}

vkcore::PipelineLayout BillboardPipelines::BuildPipelineLayout(
    const vkcore::Device& device, const GameDataBinding& gameDataBinding) {
  return vkcore::PipelineLayout(device,
                                std::to_array<const vkcore::DescriptorSetLayout*>(
                                    {&gameDataBinding.descriptorSetLayout(), &textureSetLayout_}));
}

vkcore::Pipeline BillboardPipelines::BuildPipeline(const vkcore::Device& device,
                                                   const vkcore::RenderPass& renderPass,
                                                   const ShaderCompiler& shaderCompiler,
                                                   RenderLayer renderLayer) {
  vkcore::GraphicsPipelineCreator creator(device);

  bool depthWriteEnable = (renderLayer != RenderLayer::Translucent);
  bool colorBlendEnable = (renderLayer == RenderLayer::Translucent);

  std::string fragmentPath = (renderLayer != RenderLayer::Cutout)
                                 ? core::kShadersPrefix + "billboard.frag"
                                 : core::kShadersPrefix + "billboard_cutout.frag";

  vkcore::ShaderModule vertex = CompileShaderModule(
      shaderCompiler, device, core::kShadersPrefix + "billboard.vert", shaderc_vertex_shader);
  vkcore::ShaderModule fragment =
      CompileShaderModule(shaderCompiler, device, fragmentPath, shaderc_fragment_shader);



  return creator.AddShaderStage(vertex, VK_SHADER_STAGE_VERTEX_BIT)
      .AddShaderStage(fragment, VK_SHADER_STAGE_FRAGMENT_BIT)
      .AddVertexBinding(0, sizeof(BillboardInstance), VK_VERTEX_INPUT_RATE_INSTANCE)
      .AddVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(BillboardInstance, pos))
      .AddVertexAttribute(1, 0, VK_FORMAT_R32_SFLOAT, offsetof(BillboardInstance, rotation))
      .AddVertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(BillboardInstance, size))
      .AddVertexAttribute(3, 0, VK_FORMAT_R32G32B32A32_SFLOAT,
                          offsetof(BillboardInstance, uvMinMax))
      .AddVertexAttribute(4, 0, VK_FORMAT_R32_SFLOAT, offsetof(BillboardInstance, layer))
      .AddVertexAttribute(5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(BillboardInstance, color))
      .setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
      .setCullMode(VK_CULL_MODE_NONE)
      .AddViewport(VkViewport{
          0.0f,
          0.0f,
          1280.0f,
          720.0f,
          0.0f,
          1.0f,
      })
      .AddScissor(VkRect2D{
          {0, 0},
          {1280, 720},
      })
      .setDepthTest(true, depthWriteEnable)
      .AddColorBlendAttachment(colorBlendEnable)
      .AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
      .AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
      .Build(pipelineLayout_.handle(), renderPass.handle());
}

std::array<vkcore::Pipeline, static_cast<size_t>(RenderLayer::Count)>
BillboardPipelines::BuildPipelines(const vkcore::Device& device,
                                   const vkcore::RenderPass& renderPass,
                                   const ShaderCompiler& shaderCompiler) {
  return {
      BuildPipeline(device, renderPass, shaderCompiler, RenderLayer::Solid),
      BuildPipeline(device, renderPass, shaderCompiler, RenderLayer::Cutout),
      BuildPipeline(device, renderPass, shaderCompiler, RenderLayer::Translucent),
  };
}

BillboardPipelines::BillboardPipelines(const vkcore::Device& device,
                                       const vkcore::RenderPass& renderPass,
                                       const GameDataBinding& gameDataBinding,
                                       const ShaderCompiler& shaderCompiler)
    : textureSetLayout_(BuildTextureSetLayout(device)),
      pipelineLayout_(BuildPipelineLayout(device, gameDataBinding)),
      pipelines_(BuildPipelines(device, renderPass, shaderCompiler)) {}
}  // namespace gfx