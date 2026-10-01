#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "../../../vkcore/devices/Device.h"
#include "../../../vkcore/resource/DescriptorSetLayout.h"
#include "../../../vkcore/resource/DescriptorPool.h"
#include "../../../game/voxel/BlockManager.h"
#include "../../common/texture/Atlas.h"
#include "BlockRenderGroupRegistry.h"
#include "BlockSurfaceRegistry.h"
#include "BlockUvBuffer.h"

namespace gfx {
class BlockRenderData {
 public:
  static constexpr uint32_t kFaceCount = 6;

  BlockRenderData(const gm::BlockManager& blockManager, const vkcore::Device& device,
                  vkcore::TransferContext& transferCtxt, vkcore::MemoryAllocator& memoryAllocator,
                  uint32_t framesInFlight);

  void Update(float dt, uint32_t currentFrameInFlight);

  [[nodiscard]] const UvRegion& ExtractRegion(uint32_t blockId, gm::Block::Face face) {
    return surfaceRegistry_.ExtractRegion(
        blockInfos_[blockId].surfaces[static_cast<uint32_t>(face)]);
  }

  [[nodiscard]] BlockSurfaceId GetSurfaceId(uint32_t blockId, gm::Block::Face face);
  [[nodiscard]] RenderGroupId GetRenderGroupId(uint32_t blockId);

  [[nodiscard]] const Atlas& GetAtlas() const { return atlas_; }
  [[nodiscard]] Atlas& GetAtlas() { return atlas_; }
  [[nodiscard]] const BlockSurfaceRegistry& GetSurfaceRegistry() const { return surfaceRegistry_; }
  [[nodiscard]] const RenderGroupRegistry& GetRenderGroupRegistry() const {
    return renderGroupRegistry_;
  }
  [[nodiscard]] const std::vector<BlockUvBuffer>& GetUvBuffer() const { return uvBuffers_; }

  [[nodiscard]] const vkcore::DescriptorSetLayout& GetDescriptorSetLayout() const {
    return descriptorSetLayout_;
  }
  [[nodiscard]] const vkcore::DescriptorSet& GetDescriptorSet(uint32_t frame) const {
    assert(frame < descriptorSets_.size());
    return descriptorSets_[frame];
  }

 private:
  void Build();
  [[nodiscard]] vkcore::DescriptorSetLayout BuildDescriptorSetLayout(
      const vkcore::Device& device);
  [[nodiscard]] vkcore::DescriptorPool BuildDescriptorPool(
      const vkcore::Device& device, uint32_t framesCount);
  
  void BuildDescriptors(uint32_t framesCount);
  

  struct BlockRenderInfo {
    static constexpr uint32_t kFaceCount = 6u;
    std::array<BlockSurfaceId, kFaceCount> surfaces;
    RenderGroupId renderGroup;
    BlockRenderInfo() : renderGroup(RenderGroupRegistry::kInvalid) {
      surfaces.fill(BlockSurfaceRegistry::kInvalidSurface);
    }
  };

 private:
  const gm::BlockManager* blockManager_;
  vkcore::TransferContext* transferCtxt_;
  const vkcore::Device* device_;

  Atlas atlas_;
  BlockSurfaceRegistry surfaceRegistry_;
  RenderGroupRegistry renderGroupRegistry_;
  std::vector<BlockUvBuffer> uvBuffers_;
  std::vector<BlockRenderInfo> blockInfos_;

  vkcore::DescriptorSetLayout descriptorSetLayout_;
  vkcore::DescriptorPool descriptorPool_;
  std::vector<vkcore::DescriptorSet> descriptorSets_;
};

}  // namespace gfx