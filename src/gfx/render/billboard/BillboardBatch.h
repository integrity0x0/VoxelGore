#pragma once

#include <span>
#include <vector>

#include "../../common/texture/Atlas.h"
#include "BillboardInstance.h"
#include "BillboardPipelines.h"
#include "glm/gtx/norm.hpp"

namespace gfx {

class BillboardBatch {
 public:
  enum class SortingMode {
    None,
    BackToFront,
  };

  BillboardBatch(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                 const vkcore::DescriptorSet& atlasDescriptorSet,
                 const vkcore::PipelineLayout& pipelineLayout, uint32_t framesCount,
                 SortingMode sortingMode = SortingMode::None);

  void BeginFrame(uint32_t currentFrame);

  void Submit(const BillboardInstance& billboard, uint32_t currentFrame);

  void Render(VkCommandBuffer cmd, const glm::vec3& cameraPos, uint32_t currentFrame);

  [[nodiscard]] bool IsEmpty() const { return billboardCount_ == 0; }

 private:
  static constexpr VkDeviceSize kBufferSize = 6 * 1024 * 1024ull;
  static constexpr size_t kMaxbillboards = kBufferSize / sizeof(BillboardInstance);

  static constexpr VkBufferUsageFlags kBufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  static constexpr VkMemoryPropertyFlags kMemoryProperties =
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  static constexpr uint32_t kAtlasSetIndex = 1u;

  struct BillboardSortEntry {
    uint32_t index;
    float distance;
  };

  struct FrameData {
    std::span<BillboardInstance> mapped;
    vkcore::BufferSlice instanceBuffer;

    FrameData(std::span<BillboardInstance> mapped, vkcore::BufferSlice&& instanceBuffer)
        : mapped(mapped), instanceBuffer(std::move(instanceBuffer)) {}
  };

 private:
  const vkcore::Device* device_;
  const vkcore::PipelineLayout* pipelineLayout_;
  const vkcore::DescriptorSet* descriptorSet_;

  SortingMode sortingMode_;

  std::vector<FrameData> frames_;

  uint32_t billboardCount_ = 0;

  std::vector<BillboardInstance> instances_;
  std::vector<BillboardSortEntry> sortBuffer_;
};

}  // namespace gfx