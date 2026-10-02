#pragma once

#include <glm/glm.hpp>

#include "../../../vkcore/resource/BufferAllocator.h"
#include "../../common/texture/MaterialManager.h"
#include "UiTypes.h"
#include "UiQuadInstance.h"

namespace gfx {
class UiRenderer {
 public:

  UiRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator, uint32_t framesCount);
 

  void Begin(uint32_t currentFrame) {
    assert(currentFrame < frames_.size());
    currentFrame_ = currentFrame;
    vertexOffset_ = vertexCount_ = 0;
  }

  void Submit(glm::vec2 pos, glm::vec2 size, const std::optional<UiTextureRegion>& texture, const glm::vec4& color);
  void Render(VkCommandBuffer cmd);

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
  static constexpr size_t kMaxVertices = kBufferSize / sizeof(UiQuadInstance);
  std::reference_wrapper<const vkcore::Device> device_;
  std::vector<FrameData> frames_;
  uint32_t vertexOffset_ = 0;
  uint32_t vertexCount_ = 0;
  uint32_t currentFrame_ = 0;
  MaterialId bindedMaterial_ = kInvalidMaterialId;
  UiTextureRegion blankTexture_;
};
}  // namespace gfx
