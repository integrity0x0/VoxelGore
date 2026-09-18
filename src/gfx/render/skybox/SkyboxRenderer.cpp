#include "SkyboxRenderer.h"

#include <array>

#include "../../../vkcore/resource/memoryUtils.h"
#include "SkyboxVertex.h"

namespace gfx {

namespace {
#include "skyboxCubeVertices.inl"
}  // namespace

vkcore::DescriptorPool SkyboxRenderer::BuildDescriptorPool(const vkcore::Device& device) {
  return vkcore::DescriptorPool(device, std::to_array(
    {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kMaxSets}}), kMaxSets
  );
}

Mesh SkyboxRenderer::BuildMesh(const vkcore::Device& device, 
                               vkcore::BufferAllocator& bufferAllocator,
                               vkcore::TransferContext& transferCtxt) {
  vkcore::BufferSlice bufferSlice =
      bufferAllocator.Allocate(kCubeVertices.size() * sizeof(SkyboxVertex), kBufferUsage, kMemoryProperties);
  transferCtxt.Begin();

  vkcore::LoadDataToBuffer(device, transferCtxt,
                           {reinterpret_cast<const std::byte*>(kCubeVertices.data()),
                           kCubeVertices.size() * sizeof(SkyboxVertex)},
                           bufferSlice.buffer(), bufferSlice.offset());
  transferCtxt.Flush();
  return Mesh(device, std::move(bufferSlice), static_cast<uint32_t>(kCubeVertices.size()));
}

SkyboxRenderer::SkyboxRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                               vkcore::TransferContext& transferCtxt, 
                               const vkcore::RenderPass& renderPass,
                               const GameDataBinding& gameDataBinding,
                               const ShaderCompiler& shaderCompiler) 
    : pipeline_(device, renderPass, gameDataBinding, shaderCompiler),
      descriptorPool_(BuildDescriptorPool(device)),
      mesh_(BuildMesh(device, bufferAllocator, transferCtxt)) {}

void SkyboxRenderer::BindPipeline(VkCommandBuffer cmd) const { pipeline_.Bind(cmd); }

void SkyboxRenderer::Draw(VkCommandBuffer cmd, const Skybox& skybox) const {
  mesh_.Bind(cmd);
  skybox.Bind(cmd, pipeline_.pipelineLayout().handle());
  mesh_.Draw(cmd);
}
}  // namespace gfx