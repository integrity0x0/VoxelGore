#include "ChunkRenderPipelines.h"

#include "../../../core/PathPrefixes.h"

namespace gfx {
ChunkRenderPipelines::ChunkRenderPipelines(
    const vkcore::Device& device, VkRenderPass renderPass, const GameDataBinding& gameDataBinding,
    const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout,
    const ShaderCompiler& shaderCompiler)
    : device_(&device),
      pipelineLayout_(BuildPipelineLayout(device, gameDataBinding, atlasDescriptorSetLayout)),
      pipelines_{BuildPipeline(renderPass, shaderCompiler, RenderLayer::Solid),
                 BuildPipeline(renderPass, shaderCompiler, RenderLayer::Cutout),
                 BuildPipeline(renderPass, shaderCompiler, RenderLayer::Translucent)} {}

vkcore::PipelineLayout ChunkRenderPipelines::BuildPipelineLayout(
    const vkcore::Device& device, const GameDataBinding& gameDataBinding,
    const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout) {
  return vkcore::PipelineLayout(
      device,
      std::vector<const vkcore::DescriptorSetLayout*>{&gameDataBinding.descriptorSetLayout(),
                                                      &atlasDescriptorSetLayout},
      std::vector<VkPushConstantRange>{});
}

vkcore::Pipeline ChunkRenderPipelines::BuildPipeline(VkRenderPass renderPass,
                                                     const ShaderCompiler& shaderCompiler,
                                                     RenderLayer renderLayer) const {
  bool blendEnabled = (renderLayer == RenderLayer::Translucent);
  bool depthWrite = (renderLayer != RenderLayer::Translucent);
  std::string fragmentPath = (renderLayer != RenderLayer::Cutout) ? core::kShadersPrefix + "chunks.frag"
                                                                  : core::kShadersPrefix + "chunks_cutout.frag";
  vkcore::ShaderModule vert = CompileShaderModule(
      shaderCompiler, *device_, core::kShadersPrefix + "chunks.vert", shaderc_vertex_shader);

  vkcore::ShaderModule frag = CompileShaderModule(
      shaderCompiler, *device_, fragmentPath, shaderc_fragment_shader);

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