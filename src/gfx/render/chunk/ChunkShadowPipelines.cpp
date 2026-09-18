#include "ChunkShadowPipelines.h"

#include "../../../core/PathPrefixes.h"

namespace gfx {

ChunkShadowPipelines::ChunkShadowPipelines(const vkcore::Device& device,
                                           const ShadowPass& shadowRenderPass,
                                           const GameDataBinding& gameDataBinding,
                                           const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout,
                                           const ShaderCompiler& shaderCompiler)
    : device_(&device),
      pipelineLayout_(BuildPipelineLayout(device, gameDataBinding, atlasDescriptorSetLayout)),
      pipelines_{BuildPipeline(shadowRenderPass.handle(), shaderCompiler, ShadowLayer::Solid),
                 BuildPipeline(shadowRenderPass.handle(), shaderCompiler, ShadowLayer::Cutout)} {}

vkcore::PipelineLayout ChunkShadowPipelines::BuildPipelineLayout(
    const vkcore::Device& device, const GameDataBinding& gameDataBinding,
    const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout) {
  return vkcore::PipelineLayout(
      device,
      std::vector<const vkcore::DescriptorSetLayout*>{&gameDataBinding.descriptorSetLayout(),
                                                      &atlasDescriptorSetLayout},
      std::vector<VkPushConstantRange>{});
}

vkcore::Pipeline ChunkShadowPipelines::BuildPipeline(VkRenderPass shadowRenderPass,
                                                     const ShaderCompiler& shaderCompiler,
                                                     ShadowLayer layer) const {

  const bool isCutout = (layer == ShadowLayer::Cutout);

  vkcore::ShaderModule vert = CompileShaderModule(
      shaderCompiler, *device_, core::kShadersPrefix + "chunk_shadow.vert", shaderc_vertex_shader);
  vkcore::ShaderModule frag = CompileShaderModule(
      shaderCompiler, *device_, core::kShadersPrefix + "chunk_shadow.frag", shaderc_vertex_shader);



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
      .AddColorBlendAttachment(false)
      .AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
      .AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
      .setDepthTest(true, true)
      .setCullMode(VK_CULL_MODE_FRONT_BIT)
      .Build(pipelineLayout_.handle(), shadowRenderPass);
}

void ChunkShadowPipelines::Bind(VkCommandBuffer cmd, ShadowLayer layer) const {
  assert(layer < ShadowLayer::Count);
  pipelines_[static_cast<size_t>(layer)].Bind(cmd);
}

}  // namespace gfx