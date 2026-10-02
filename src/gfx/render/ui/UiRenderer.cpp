#include "UiRenderer.h"

namespace gfx {
UiRenderer::UiRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                       uint32_t framesCount) 
	: device_(device) {
  frames_.reserve(framesCount);
  for (size_t i = 0; i < static_cast<size_t>(framesCount); ++i) {
    vkcore::BufferSlice bufferSlice =
        bufferAllocator.Allocate(kBufferSize, kBufferUsage, kMemoryProperties);
    UiQuadInstance* mapped = reinterpret_cast<UiQuadInstance*>(bufferSlice.Map());
    frames_.emplace_back(std::move(bufferSlice), mapped);
  }
}

void UiRenderer::Submit(glm::vec2 pos, glm::vec2 size,
                        const std::optional<UiTextureRegion>& texture, const glm::vec4& color) {
  if (vertexOffset_ + vertexCount_ + 1 > kMaxVertices) return;

  auto& frame = frames_[currentFrame_];
  UiQuadInstance* v = frame.mapped + (vertexOffset_ + vertexCount_);

  const UiTextureRegion& region = texture ? *texture : blankTexture_;

  const glm::vec2 uvMin = region.region.min;
  const glm::vec2 uvMax = region.region.max;

  v[0] = {
      pos,
      size,
      {uvMin.x, uvMin.y, uvMax.x, uvMax.y},
      color,
      0.0f  // radius
  };

  vertexCount_ += 1;
}

void UiRenderer::Render(VkCommandBuffer cmd) {
  auto& frame = frames_[currentFrame_];

  frame.vertexBuffer.BindVertex(cmd);

  device_.get().GetDispatchTable().vkCmdDraw(cmd, vertexCount_, 1, 0, 0);

  vertexOffset_ += vertexCount_;
  vertexCount_ = 0;
}
}  // namespace gfx