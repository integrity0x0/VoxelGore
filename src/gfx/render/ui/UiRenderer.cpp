#include "UiRenderer.h"

namespace gfx {
UiRenderer::UiRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                       MaterialManager& materialManager,
                       const vkcore::RenderPass& renderPass,
                       const ShaderCompiler& shaderCompiler,
                       const UiTextureRegion& blankTexture,
                       uint32_t framesCount) 
	: device_(device),
    materialManager_(materialManager),
    pipeline_(device, materialManager.GetDescriptorSetLayout(), renderPass, shaderCompiler),
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
                        float radius, float borderWidth) {
  if (static_cast<size_t>(instanceOffset_ + instanceCount_ + 1) > kMaxInstances) return;

  auto& frame = frames_[currentFrame_];

  const bool hasValidTexture = texture && texture->materialId != kInvalidMaterialId;
  const UiTextureRegion& region = hasValidTexture ? *texture : blankTexture_;

  if (region.materialId != bindedMaterial_) {
    if (instanceCount_ > 0) Render();

    materialManager_.get()
        .Get(region.materialId)
        .descriptorSet.Bind(cmd_, pipeline_.GetLayout().GetHandle());

    bindedMaterial_ = region.materialId;
  }

  const glm::vec2 uvMin = region.region.min;
  const glm::vec2 uvMax = region.region.max;

  UiQuadInstance& instance = *(frame.mapped + (instanceOffset_ + instanceCount_));

  instance = {.pos = pos,
              .size = size,
              .uvRect = {uvMin.x, uvMin.y, uvMax.x, uvMax.y},
              .color = color,
              .radius = radius,
              .borderWidth = borderWidth,
              .type = UiQuadInstance::Type::Shape};

  ++instanceCount_;
}

void UiRenderer::SubmitGlyph(glm::vec2 pos, glm::vec2 size, const std::optional<GlyphSlot>& slot,
                             const glm::vec4& color) {
  if (static_cast<size_t>(instanceOffset_ + instanceCount_ + 1) > kMaxInstances) return;
  if (!slot || slot->materialId == kInvalidMaterialId) return;

  auto& frame = frames_[currentFrame_];

  if (slot->materialId != bindedMaterial_) {
    if (instanceCount_ > 0) Render();

    materialManager_.get()
        .Get(slot->materialId)
        .descriptorSet.Bind(cmd_, pipeline_.GetLayout().GetHandle());

    bindedMaterial_ = slot->materialId;
  }

  const glm::vec2 uvMin = slot->region.min;
  const glm::vec2 uvMax = slot->region.max;

  UiQuadInstance& instance = *(frame.mapped + (instanceOffset_ + instanceCount_));

  instance = {.pos = pos,
              .size = size,
              .uvRect = {uvMin.x, uvMin.y, uvMax.x, uvMax.y},
              .color = color,
              .radius = 0.0f,
              .borderWidth = -1.0f,
              .type = UiQuadInstance::Type::Glyph};

  ++instanceCount_;
}

void UiRenderer::Render() {
  auto& frame = frames_[currentFrame_];

  device_.get().GetDispatchTable().vkCmdDraw(cmd_, 6, instanceCount_, 0, instanceOffset_);

  instanceOffset_ += instanceCount_;
  instanceCount_ = 0;
}
}  // namespace gfx