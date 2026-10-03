#include "UiRenderer.h"

namespace gfx {
UiRenderer::UiRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                       const vkcore::DescriptorSetLayout& materialLayout,
                       const vkcore::RenderPass& renderPass,
                       const ShaderCompiler& shaderCompiler,
                       const UiTextureRegion& blankTexture,
                       uint32_t framesCount) 
	: device_(device),
    pipeline_(device, materialLayout, renderPass, shaderCompiler),
    blankTexture_(blankTexture) {
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
  if (static_cast<size_t>(instanceOffset_ + instanceCount_ + 1) > kMaxInstances) return;

  auto& frame = frames_[currentFrame_];
  
  const UiTextureRegion& region = texture ? *texture : blankTexture_;

  if (region.material.get().id != bindedMaterial_) {
    region.material.get().descriptorSet.Bind(cmd_, pipeline_.GetLayout().GetHandle());
  }

  const glm::vec2 uvMin = region.region.min;
  const glm::vec2 uvMax = region.region.max;

  UiQuadInstance& instance = *(frame.mapped + (instanceOffset_ + instanceCount_));

  instance = {
      .pos = pos,
      .size = size,
      .uvRect = {uvMin.x, uvMin.y, uvMax.x, uvMax.y},
      .color = color,
      .radius = 0.0f
  };

  ++instanceCount_;
}

void UiRenderer::Render() {
  auto& frame = frames_[currentFrame_];

  frame.vertexBuffer.BindVertex(cmd_);

  device_.get().GetDispatchTable().vkCmdDraw(cmd_, 6, instanceCount_, 0, 0);

  instanceOffset_ += instanceCount_;
  instanceCount_ = 0;
}
}  // namespace gfx