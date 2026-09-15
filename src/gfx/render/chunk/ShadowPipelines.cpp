#include "ShadowPipelines.h"

#include "../../../core/PathPrefixes.h"

namespace gfx::render::chunk {

ShadowPipelines::ShadowPipelines(const vkcore::Device& device,
                                 const shadow::ShadowPass& shadowRenderPass,
                                 const GameDataBinding& gameDataBinding,
                                 const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout)
    : device_(&device),
      pipelineLayout_(BuildPipelineLayout(device, gameDataBinding, atlasDescriptorSetLayout)),
      pipelines_{BuildPipeline(shadowRenderPass.handle(), ShadowLayer::Solid),
                 BuildPipeline(shadowRenderPass.handle(), ShadowLayer::Cutout)} {}

vkcore::PipelineLayout ShadowPipelines::BuildPipelineLayout(
    const vkcore::Device& device, const GameDataBinding& gameDataBinding,
    const vkcore::DescriptorSetLayout& atlasDescriptorSetLayout) {
  // Тот же layout, что и у обычных чанков (GameData + Atlas).
  // Для Solid atlas не обязателен, но так проще и совместимее.
  return vkcore::PipelineLayout(
      device,
      std::vector<const vkcore::DescriptorSetLayout*>{&gameDataBinding.descriptorSetLayout(),
                                                      &atlasDescriptorSetLayout},
      std::vector<VkPushConstantRange>{});
}

vkcore::Pipeline ShadowPipelines::BuildPipeline(VkRenderPass shadowRenderPass,
                                                ShadowLayer layer) const {

  const bool isCutout = (layer == ShadowLayer::Cutout);

  const std::string vertPath = isCutout ? core::kAssetsPrefix + "shaders/main_depth.vert.spv"
                                        : core::kAssetsPrefix + "shaders/main_depth_cutout.vert.spv";


  const std::string fragPath = isCutout ? core::kAssetsPrefix + "shaders/main_depth_cutout.frag.spv"
                                        : core::kAssetsPrefix + "shaders/main_depth.frag.spv";

  return vkcore::GraphicsPipelineCreator(*device_)
      .AddShaderStage(vertPath, VK_SHADER_STAGE_VERTEX_BIT)
      .AddShaderStage(core::kAssetsPrefix + fragPath, VK_SHADER_STAGE_FRAGMENT_BIT)
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

void ShadowPipelines::Bind(VkCommandBuffer cmd, ShadowLayer layer) const {
  assert(layer < ShadowLayer::Count);
  pipelines_[static_cast<size_t>(layer)].Bind(cmd);
}

}  // namespace gfx::render::chunk