#include "ChunkRenderPipelines.h"

#include "../../../core/PathPrefixes.h"
#include "../../common/shader/ShaderDefinitions.h"

namespace gfx {
ChunkRenderPipelines::ChunkRenderPipelines(
    const vkcore::Device& device, VkRenderPass renderPass, const GameDataBinding& gameDataBinding,
    const vkcore::DescriptorSetLayout* shadowLayout,
    const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout,
    const ShaderCompiler& shaderCompiler)
    : device_(&device),
      pipelineLayout_(BuildPipelineLayout(device, gameDataBinding, shadowLayout, atlasDescriptorSetLayout)),
      pipelines_{BuildPipeline(renderPass, shaderCompiler, RenderLayer::Solid),
                 BuildPipeline(renderPass, shaderCompiler, RenderLayer::Cutout),
                 BuildPipeline(renderPass, shaderCompiler, RenderLayer::Translucent)} {}

vkcore::PipelineLayout ChunkRenderPipelines::BuildPipelineLayout(
    const vkcore::Device& device, const GameDataBinding& gameDataBinding,
    const vkcore::DescriptorSetLayout* shadowLayout,
    const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout) {
  std::vector<const vkcore::DescriptorSetLayout*> layouts = {&gameDataBinding.descriptorSetLayout(),
                                                             &atlasDescriptorSetLayout};
  if (shadowLayout) layouts.emplace_back(shadowLayout);

  return vkcore::PipelineLayout(
      device,
      layouts,
      {});
}

vkcore::Pipeline ChunkRenderPipelines::BuildPipeline(VkRenderPass renderPass,                                                   
                                                     const ShaderCompiler& shaderCompiler,
                                                     RenderLayer renderLayer) const {
  bool blendEnabled = (renderLayer == RenderLayer::Translucent);
  bool depthWrite = (renderLayer != RenderLayer::Translucent);

  ShaderDefinitions definitions = (renderLayer != RenderLayer::Cutout)
                                      ? ShaderDefinitions{}
                                      : ShaderDefinitions{{kShaderCutoutLayerDefinition, "1"}};

  vkcore::ShaderModule vert = CompileShaderModule(shaderCompiler, *device_, core::kShadersPrefix + "chunk.vert",
                          shaderc_vertex_shader, definitions);

  vkcore::ShaderModule frag =
      CompileShaderModule(shaderCompiler, *device_, core::kShadersPrefix + "chunk.frag",
                                                  shaderc_fragment_shader, definitions);

  return vkcore::GraphicsPipelineCreator(*device_)
      .AddShaderStage(vert, VK_SHADER_STAGE_VERTEX_BIT)
      .AddShaderStage(frag, VK_SHADER_STAGE_FRAGMENT_BIT)
      .AddVertexBinding(0, sizeof(ChunkMeshBuilder::Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
      .AddVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ChunkMeshBuilder::Vertex, pos))
      .AddVertexAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT,
                          offsetof(ChunkMeshBuilder::Vertex, color))
      .AddVertexAttribute(2, 0, VK_FORMAT_R8_UINT, offsetof(ChunkMeshBuilder::Vertex, faceIndex))
      .AddVertexAttribute(3, 0, VK_FORMAT_R8_UINT, offsetof(ChunkMeshBuilder::Vertex, cornerIndex))
      .AddVertexAttribute(4, 0, VK_FORMAT_R32_UINT,
                          offsetof(ChunkMeshBuilder::Vertex, blockSurfaceId))
      .AddColorBlendAttachment(blendEnabled)
      .AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
      .AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
      .setDepthTest(true, depthWrite)
      .setCullMode(VK_CULL_MODE_NONE)
      .Build(pipelineLayout_.handle(), renderPass);
}

void ChunkRenderPipelines::Bind(VkCommandBuffer cmd, RenderLayer renderLayer) const {
  assert(renderLayer < RenderLayer::Count);
  pipelines_[static_cast<size_t>(renderLayer)].Bind(cmd);
}

}  // namespace gfx