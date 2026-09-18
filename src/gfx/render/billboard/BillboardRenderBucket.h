#pragma once

#include <array>
#include <glm/vec3.hpp>

#include "../../common/texture/Atlas.h"
#include "BillboardBatch.h"
#include "BillboardPipelines.h"

namespace gfx {

class BillboardRenderBucket {
 public:
  BillboardRenderBucket(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                        const BillboardPipelines& pipelines, const Atlas& atlas,
                        uint32_t framesCount);

  void Submit(const BillboardInstance& billboard, RenderLayer renderLayer, uint32_t currentFrame);
  void Render(VkCommandBuffer cmd, const glm::vec3& cameraPos, RenderLayer renderLayer,
              uint32_t currentFrame);

 private:
  [[nodiscard]] vkcore::DescriptorPool BuildDescriptorPool(const vkcore::Device& device);
  [[nodiscard]] vkcore::DescriptorSet AllocateDescriptorSet(const BillboardPipelines& pipelines);
  [[nodiscard]] std::array<BillboardBatch, static_cast<size_t>(RenderLayer::Count)> BuildBatches(
      const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator, uint32_t framesCount);

 private:
  static constexpr uint32_t kAtlasSetIndex = 1u;

  const BillboardPipelines* pipelines_;
  const Atlas* atlas_;

  vkcore::DescriptorPool descriptorPool_;
  vkcore::DescriptorSet descriptorSet_;

  std::array<BillboardBatch, static_cast<size_t>(RenderLayer::Count)> batches_;

  uint32_t currentFrame_ = 0;
};
}  // namespace gfx