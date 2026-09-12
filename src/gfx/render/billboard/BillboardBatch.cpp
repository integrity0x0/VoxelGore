#include "BillboardBatch.h"

#include <algorithm>

namespace gfx {

BillboardBatch::BillboardBatch(const vkcore::Device& device,
                               vkcore::BufferAllocator& bufferAllocator,
                               const vkcore::DescriptorSet& atlasDescriptorSet,
                               const vkcore::PipelineLayout& pipelineLayout, uint32_t framesCount,
                               SortingMode sortingMode)
    : device_(&device),
      descriptorSet_(&atlasDescriptorSet),
      pipelineLayout_(&pipelineLayout),
      sortingMode_(sortingMode) {
  frames_.reserve(framesCount);

  if (sortingMode_ == SortingMode::BackToFront) {
    instances_.reserve(kMaxbillboards);
    sortBuffer_.reserve(kMaxbillboards);
  }

  for (uint32_t i = 0; i < framesCount; ++i) {
    auto buffer = bufferAllocator.Allocate(kBufferSize, kBufferUsage, kMemoryProperties);

    auto mapped = std::span<BillboardInstance>(reinterpret_cast<BillboardInstance*>(buffer.map()),
                                               kMaxbillboards);

    frames_.emplace_back(mapped, std::move(buffer));
  }
}

void BillboardBatch::Submit(const BillboardInstance& billboard, uint32_t currentFrame) {
  if (billboardCount_ >= kMaxbillboards) {
    return;
  }

  if (sortingMode_ == SortingMode::None) {
    frames_[currentFrame].mapped[billboardCount_] = billboard;
  } else {
    instances_.push_back(billboard);
  }

  ++billboardCount_;
}

void BillboardBatch::Render(VkCommandBuffer cmd, const glm::vec3& cameraPosition,
                            uint32_t currentFrame) {
  if (billboardCount_ == 0) {
    return;
  }

  FrameData& frame = frames_[currentFrame];

  if (sortingMode_ == SortingMode::BackToFront) {
    sortBuffer_.clear();

    for (uint32_t i = 0; i < billboardCount_; ++i) {
      sortBuffer_.push_back({
          i,
          glm::length2(instances_[i].pos - cameraPosition),
      });
    }

    std::sort(sortBuffer_.begin(), sortBuffer_.end(),
              [](const BillboardSortEntry& a, const BillboardSortEntry& b) {
                return a.distance > b.distance;
              });

    for (uint32_t i = 0; i < billboardCount_; ++i) {
      frame.mapped[i] = instances_[sortBuffer_[i].index];
    }
  }

  const auto& dt = device_->dispatchTable();

  VkDescriptorSet sets[] = {
      descriptorSet_->handle(),
  };

  dt.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_->handle(),
                             kAtlasSetIndex, 1, sets, 0, nullptr);

  VkBuffer buffers[] = {frame.instanceBuffer.handle()};
  VkDeviceSize offsets[] = {frame.instanceBuffer.offset()};

  dt.vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);

  dt.vkCmdDraw(cmd, 4, billboardCount_, 0, 0);

  if (sortingMode_ != SortingMode::None) {
    instances_.clear();
    sortBuffer_.clear();
  }
  billboardCount_ = 0;
}

}  // namespace gfx