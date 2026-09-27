#pragma once

#include <glm/glm.hpp>

#include "../../../vkcore/resource/BufferAllocator.h"
#include "../../common/texture/MaterialManager.h"

namespace gfx {
class UiRenderer {
 public:
  static constexpr VkDeviceSize kBufferSize = 32 * 1024 * 1024ull;
  struct Vertex {
    glm::vec2 pos;
    glm::vec2 uv;
    glm::vec4 color;
  };

  UiRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator);
 private:
  struct FrameData {
    vkcore::BufferSlice vertexBuffer;
    Vertex* mapped;
  };
 private:
  std::reference_wrapper<const vkcore::Device> device_;
  std::vector<vkcore::BufferSlice> vertexBuffers_;
  MaterialId bindedMaterial_ = kInvalidMaterialId;
};
}  // namespace gfx
