#include "ModelRenderer.h"

#include <cstring>

namespace gfx {

ModelRenderer::ModelRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                             const ModelPipeline& pipeline, uint32_t framesCount)
    : device_(&device), bufferAllocator_(&bufferAllocator), pipeline_(&pipeline) {
  frames_.reserve(framesCount);

  for (uint32_t i = 0; i < framesCount; ++i) {
    vkcore::BufferSlice buffer = bufferAllocator.Allocate(
        kMaxInstances * sizeof(ModelInstanceData), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto* mapped = reinterpret_cast<ModelInstanceData*>(buffer.map());

    frames_.emplace_back(std::move(buffer), mapped);
  }
}

void ModelRenderer::Submit(const Model& model, const glm::mat4x3& transform,
                           const glm::vec4& color) {
  if (instanceCount_ >= kMaxInstances) {
    return;
  }

  ModelId id = model.id();

  if (id >= sparseIndices_.size()) {
    sparseIndices_.resize(id + 1, kInvalidGroup);
  }

  uint32_t groupIndex = sparseIndices_[id];

  if (groupIndex == kInvalidGroup) {
    groupIndex = static_cast<uint32_t>(groups_.size());

    sparseIndices_[id] = groupIndex;
    groups_.push_back({&model, {}});
  }

  groups_[groupIndex].instances.emplace_back(transform, color);
  ++instanceCount_;
}

void ModelRenderer::Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
  FrameData& frame = frames_[currentFrame];

  uint32_t instanceOffset = 0;

  for (auto& group : groups_) {
    uint32_t count = static_cast<uint32_t>(group.instances.size());

    if (instanceOffset + count > kMaxInstances) {
      break;
    }

    std::memcpy(frame.mapped + instanceOffset, group.instances.data(),
                count * sizeof(ModelInstanceData));

    group.model->Draw(commandBuffer, pipeline_->pipelineLayout(), frame.instanceBuffer.handle(),
                      instanceOffset + frame.instanceBuffer.offset(), count);

    instanceOffset += count;
  }

  groups_.clear();
  sparseIndices_.clear();
  instanceCount_ = 0;
}

}  // namespace gfx