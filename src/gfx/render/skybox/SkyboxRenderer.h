#pragma once

#include "SkyboxPipeline.h"
#include "../../common/texture/Skybox.h"
#include "../../common/mesh/Mesh.h"

namespace gfx {
class SkyboxRenderer {
 public:
  SkyboxRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                 vkcore::TransferContext& transferCtxt, const vkcore::RenderPass& renderPass,
                 const GameDataBinding& gameDataBinding, const ShaderCompiler& shaderCompiler);

  void Draw(VkCommandBuffer cmd, const Skybox& skybox) const;
  void BindPipeline(VkCommandBuffer cmd) const;
  [[nodiscard]] const vkcore::DescriptorPool& descriptorPool() const { return descriptorPool_; }
  [[nodiscard]] const vkcore::DescriptorSetLayout& descriptorSetLayout() const { return pipeline_.descriptorSetLayout(); }
 private:
  [[nodiscard]] vkcore::DescriptorPool BuildDescriptorPool(const vkcore::Device& device);
  [[nodiscard]] Mesh BuildMesh(const vkcore::Device& device,
                               vkcore::BufferAllocator& bufferAllocator,
                               vkcore::TransferContext& transferCtxt);
 private:
  static constexpr uint32_t kMaxSets = 8u;
  static constexpr VkBufferUsageFlags kBufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  static constexpr VkMemoryPropertyFlags kMemoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  SkyboxPipeline pipeline_;
  vkcore::DescriptorPool descriptorPool_;
  Mesh mesh_;
};
}  // namespace gfx