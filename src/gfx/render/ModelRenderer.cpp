#include "ModelRenderer.h"

namespace gfx {
ModelRenderer::ModelRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                             const ModelPipeline& pipeline, uint32_t framesCount)
    : device_(&device), bufferAllocator_(&bufferAllocator), pipeline_(&pipeline) {
  frames_.reserve(framesCount);

  for (size_t i = 0; i < static_cast<size_t>(framesCount); ++i) {
    vkcore::BufferSlice buffer = bufferAllocator.Allocate(
        kMaxInstances * sizeof(ModelInstanceData), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    ModelInstanceData* mapped = reinterpret_cast<ModelInstanceData*>(buffer.map());

    frames_.emplace_back(std::move(buffer), mapped);
  }
}
void ModelRenderer::BeginFrame(uint32_t currentFrame) {
  currentFrame_ = currentFrame;
  items_.clear();
  instanceCount_ = 0u;
}

void ModelRenderer::Submit(const Model& model, const glm::mat4x3& transform,
                           const glm::vec4& color) {
  if (instanceCount_ >= kMaxInstances) return;
  items_[&model].emplace_back(transform, color);
  instanceCount_++;
}

void ModelRenderer::Render(VkCommandBuffer commandBuffer) {
  auto& frame = frames_[currentFrame_];

  uint32_t instanceOffset = 0;

  for (auto& [model, instances] : items_) {
    uint32_t count = static_cast<uint32_t>(instances.size());

    if (instanceOffset + count > kMaxInstances) break;

    std::memcpy(frame.mapped + instanceOffset, instances.data(), count * sizeof(ModelInstanceData));

    model->Draw(commandBuffer, pipeline_->pipelineLayout(), frame.instanceBuffer.handle(),
                instanceOffset + frame.instanceBuffer.offset(), count);

    instanceOffset += count;
  }
}
}  // namespace gfx