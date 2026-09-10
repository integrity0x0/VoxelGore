#include "BillboardRenderer.h"

namespace gfx {
BillboardRenderer::BillboardRenderer(const vkcore::Device& device,
                                     const vkcore::RenderPass& renderPass,
                                     const GameDataBinding& gameDataBinding)
    : device_(&device), pipelines_(device, renderPass, gameDataBinding) {}

BillboardRenderBucket& BillboardRenderer::CreateBucket(vkcore::BufferAllocator& bufferAllocator,
                                                       const Atlas& atlas, uint32_t framesCount) {
  buckets_.emplace_back(std::make_unique<BillboardRenderBucket>(*device_, bufferAllocator,
                                                                pipelines_, atlas, framesCount));

  return *buckets_.back();
}

void BillboardRenderer::Render(VkCommandBuffer cmd, const glm::vec3& cameraPos,
                               RenderLayer renderLayer, uint32_t currentFrame) {
  pipelines_.Bind(cmd, renderLayer);
  for (size_t i = 0; i < buckets_.size(); ++i)
    buckets_[i]->Render(cmd, cameraPos, renderLayer, currentFrame);
}
}  // namespace gfx