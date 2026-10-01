#include "UiRenderer.h"

namespace gfx {
UiRenderer::UiRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                       uint32_t framesCount) 
	: device_(device) {
  frames_.reserve(framesCount);
  for (size_t i = 0; i < static_cast<size_t>(framesCount); ++i) {
    vkcore::BufferSlice bufferSlice =
        bufferAllocator.Allocate(kBufferSize, kBufferUsage, kMemoryProperties);
    UiVertex* mapped = reinterpret_cast<UiVertex*>(bufferSlice.Map());
    frames_.emplace_back(std::move(bufferSlice), mapped);
  }
}

void UiRenderer::Submit(glm::vec2 pos, glm::vec2 size,
                        const std::optional<UiTextureRegion>& texture,
                        const glm::vec4& color) {
  if (vertexOffset_ + vertexCount_ + 6 > kMaxVertices) return;

  auto& frame = frames_[currentFrame_];
  UiVertex* v = frame.mapped + (vertexOffset_ + vertexCount_);

  const UiTextureRegion& region = texture ? *texture : blankTexture_;
  const glm::vec2 uvMin = region.region.min;
  const glm::vec2 uvMax = region.region.max;

  const float x0 = pos.x;
  const float y0 = pos.y;
  const float x1 = pos.x + size.x;
  const float y1 = pos.y + size.y;

  v[0] = {{x0, y0}, {uvMin.x, uvMin.y}, color};  // TL
  v[1] = {{x0, y1,}, {uvMin.x, uvMax.y}, color};  // BL
  v[2] = {{x1, y1}, {uvMax.x, uvMax.y}, color};  // BR

  v[3] = {{x0, y0}, {uvMin.x, uvMin.y}, color};  // TL
  v[4] = {{x1, y1}, {uvMax.x, uvMax.y}, color};  // BR
  v[5] = {{x1, y0}, {uvMax.x, uvMin.y}, color};  // TR

  vertexCount_ += 6;
}

void UiRenderer::Render(VkCommandBuffer cmd) {
  auto& frame = frames_[currentFrame_];

  frame.vertexBuffer.BindVertex(cmd);

  device_.get().GetDispatchTable().vkCmdDraw(cmd, vertexCount_, 1, 0, 0);

  vertexOffset_ += vertexCount_;
  vertexCount_ = 0;
}
}  // namespace gfx