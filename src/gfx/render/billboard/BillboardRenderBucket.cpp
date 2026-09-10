#include "BillboardRenderBucket.h"

namespace gfx {

vkcore::DescriptorPool BillboardRenderBucket::BuildDescriptorPool(const vkcore::Device& device) {
  return vkcore::DescriptorPool(
      device, std::to_array({VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1u}}),
      1u);
}

vkcore::DescriptorSet BillboardRenderBucket::AllocateDescriptorSet(
    const BillboardPipelines& pipelines) {
  return descriptorPool_.Allocate(pipelines_->textureSetLayout());
}

std::array<BillboardBatch, static_cast<size_t>(RenderLayer::Count)>
BillboardRenderBucket::BuildBatches(const vkcore::Device& device,
                                    vkcore::BufferAllocator& bufferAllocator,
                                    uint32_t framesCount) {
  return {
      BillboardBatch(device, bufferAllocator, descriptorSet_, pipelines_->pipelineLayout(),
                     framesCount),
      BillboardBatch(device, bufferAllocator, descriptorSet_, pipelines_->pipelineLayout(),
                     framesCount),
      BillboardBatch(device, bufferAllocator, descriptorSet_, pipelines_->pipelineLayout(),
                     framesCount, gfx::BillboardBatch::SortingMode::BackToFront),
  };
}

BillboardRenderBucket::BillboardRenderBucket(const vkcore::Device& device,
                                             vkcore::BufferAllocator& bufferAllocator,
                                             const BillboardPipelines& pipelines,
                                             const Atlas& atlas, uint32_t framesCount)
    : pipelines_(&pipelines),
      atlas_(&atlas),
      descriptorPool_(BuildDescriptorPool(device)),
      descriptorSet_(AllocateDescriptorSet(pipelines)),
      batches_(BuildBatches(device, bufferAllocator, framesCount)) {
  VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.dstSet = descriptorSet_.handle();
  write.descriptorCount = 1u;
  VkDescriptorImageInfo imageInfo;
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = atlas.texture().imageView().handle();
  imageInfo.sampler = atlas.texture().sampler().handle();
  write.pImageInfo = &imageInfo;

  device.dispatchTable().vkUpdateDescriptorSets(device.handle(), 1u, &write, 0u, nullptr);
}

void BillboardRenderBucket::Submit(const BillboardInstance& billboard, RenderLayer renderLayer,
                                   uint32_t currentFrame) {
  assert(renderLayer < RenderLayer::Count);
  batches_[static_cast<size_t>(renderLayer)].Submit(billboard, currentFrame);
}

void BillboardRenderBucket::Render(VkCommandBuffer cmd, const glm::vec3& cameraPos,
                                   RenderLayer renderLayer, uint32_t currentFrame) {
  assert(renderLayer < RenderLayer::Count);
  auto& batch = batches_[static_cast<size_t>(renderLayer)];
  if (batch.IsEmpty()) return;
  batch.Render(cmd, cameraPos, currentFrame);
}

}  // namespace gfx