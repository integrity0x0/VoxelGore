#pragma once

#include <glm/glm.hpp>

#include "../../../vkcore/resource/BufferAllocator.h"
#include "../../common/texture/MaterialManager.h"
#include "../../common/shader/ShaderCompiler.h"
#include "UiTypes.h"
#include "UiRenderPipeline.h"
#include "UiQuadInstance.h"

namespace gfx {
class UiRenderer {
 public:
  UiRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
             const vkcore::DescriptorSetLayout& materialLayout,
             const vkcore::RenderPass& renderPass,
             const ShaderCompiler& shaderCompiler,
             const UiTextureRegion& blankTexture, uint32_t framesCount);

  void BeginFrame(VkCommandBuffer cmd, uint32_t currentFrame) {
    assert(currentFrame < frames_.size());
    cmd_ = cmd;
    currentFrame_ = currentFrame;
    instanceOffset_ = instanceCount_ = 0;

    pipeline_.Bind(cmd);
  }

  void Submit(glm::vec2 pos, glm::vec2 size, const std::optional<UiTextureRegion>& texture, const glm::vec4& color);
  void Render();

private:
  struct FrameData {
    vkcore::BufferSlice vertexBuffer;
    UiQuadInstance* mapped;

    FrameData(vkcore::BufferSlice&& vertexBuffer, UiQuadInstance* mapped)
        : vertexBuffer(std::move(vertexBuffer)), mapped(mapped) {}
  };

 private:
  static constexpr VkBufferUsageFlags kBufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  static constexpr VkMemoryPropertyFlags kMemoryProperties =
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  static constexpr VkDeviceSize kBufferSize = 32 * 1024 * 1024ull;
  static constexpr size_t kMaxInstances = kBufferSize / sizeof(UiQuadInstance);
  std::reference_wrapper<const vkcore::Device> device_;
  UiRenderPipeline pipeline_;
  UiTextureRegion blankTexture_;
  std::vector<FrameData> frames_;
  uint32_t instanceOffset_ = 0;
  uint32_t instanceCount_ = 0;
  VkCommandBuffer cmd_ = VK_NULL_HANDLE;
  uint32_t currentFrame_ = 0;
  MaterialId bindedMaterial_ = kInvalidMaterialId;
};
}  // namespace gfx
