#include "ModelRenderer.h"

#include <cstring>

namespace gfx {

ModelRenderer::ModelRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                             const vkcore::RenderPass& renderPass,
                             const GameDataBinding& gameDataBinding,
                             const vkcore::DescriptorSetLayout& materialSetLayout,
                             const ShadowContext* shadowCtxt, const ShaderCompiler& shaderCompiler,
                             uint32_t framesCount)
    : device_(&device), bufferAllocator_(&bufferAllocator), pipeline_(device, renderPass, gameDataBinding, materialSetLayout, shadowCtxt, shaderCompiler),
      shadowCtxt_(shadowCtxt) {
  frames_.reserve(framesCount);

  if (shadowCtxt) {
    shadowPipeline_.emplace(device, gameDataBinding, materialSetLayout,
                            *shadowCtxt, shaderCompiler);
  }

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

void ModelRenderer::UploadInstances(uint32_t frameIndex) {
  FrameData& frame = frames_[frameIndex];
  uint32_t offset = 0;
  for (auto& group : groups_) {
    uint32_t count = static_cast<uint32_t>(group.instances.size());
    if (offset + count > kMaxInstances) break;
    std::memcpy(frame.mapped + offset, group.instances.data(), count * sizeof(ModelInstanceData));
    offset += count;
  }
}

void ModelRenderer::DrawGroups(VkCommandBuffer cmd, uint32_t frameIndex,
                               const vkcore::PipelineLayout& layout) {
  FrameData& frame = frames_[frameIndex];
  uint32_t offset = 0;
  for (auto& group : groups_) {
    uint32_t count = static_cast<uint32_t>(group.instances.size());
    if (offset + count > kMaxInstances) break;
    group.model->Draw(cmd, layout, frame.instanceBuffer.handle(),
                      offset + frame.instanceBuffer.offset(), count);
    offset += count;
  }
}

void ModelRenderer::Render(VkCommandBuffer cmd, uint32_t frameIndex) {
  pipeline_.Bind(cmd);
  shadowCtxt_->descriptorSet().Bind(cmd, pipeline_.pipelineLayout().handle(), 2);
  DrawGroups(cmd, frameIndex, pipeline_.pipelineLayout());

  groups_.clear();
  sparseIndices_.clear();
  instanceCount_ = 0;
}

void ModelRenderer::RenderShadow(VkCommandBuffer cmd, uint32_t frameIndex) {
  if (!shadowPipeline_) return;
  shadowPipeline_->Bind(cmd);
  DrawGroups(cmd, frameIndex, shadowPipeline_->pipelineLayout());
}

}  // namespace gfx