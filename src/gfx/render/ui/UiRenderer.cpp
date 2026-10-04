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

void UiRenderer::BeginFrame(VkCommandBuffer cmd, uint32_t currentFrame) {
  assert(currentFrame < frames_.size());
  cmd_ = cmd;
  currentFrame_ = currentFrame;
  instanceOffset_ = instanceCount_ = 0;
  bindedMaterial_ = kInvalidMaterialId;
  pipeline_.Bind(cmd);

  frames_[currentFrame].vertexBuffer.BindVertex(cmd_);
}

void UiRenderer::Submit(glm::vec2 pos, glm::vec2 size,
                        const std::optional<UiTextureRegion>& texture, const glm::vec4& color,
                        float radius) {
  if (static_cast<size_t>(instanceOffset_ + instanceCount_ + 1) > kMaxInstances) return;

  auto& frame = frames_[currentFrame_];

  const bool hasValidTexture = texture && texture->material.get().id != kInvalidMaterialId;
  const UiTextureRegion& region = hasValidTexture ? *texture : blankTexture_;

  if (region.material.get().id != bindedMaterial_) {
    if (instanceCount_ > 0) Render();

    region.material.get().descriptorSet.Bind(cmd_, pipeline_.GetLayout().GetHandle());

    bindedMaterial_ = region.material.get().id;
  }

  const glm::vec2 uvMin = region.region.min;
  const glm::vec2 uvMax = region.region.max;

  UiQuadInstance& instance = *(frame.mapped + (instanceOffset_ + instanceCount_));

  instance = {
      .pos = pos,
      .size = size,
      .uvRect = {uvMin.x, uvMin.y, uvMax.x, uvMax.y},
      .color = color,
      .radius = radius,
  };

  ++instanceCount_;
}

void UiRenderer::Render() {
  auto& frame = frames_[currentFrame_];

  device_.get().GetDispatchTable().vkCmdDraw(cmd_, 6, instanceCount_, 0, instanceOffset_);

  instanceOffset_ += instanceCount_;
  instanceCount_ = 0;
}
}  // namespace gfx