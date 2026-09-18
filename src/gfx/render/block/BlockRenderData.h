#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "../../../game/voxel/BlockManager.h"
#include "../../../vkcore/devices/Device.h"
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

  [[nodiscard]] BlockSurfaceId surfaceId(uint32_t blockId, gm::Block::Face face);

  [[nodiscard]] RenderGroupId renderGroupId(uint32_t blockId);

  [[nodiscard]] const Atlas& atlas() const { return atlas_; }

  [[nodiscard]] Atlas& atlas() { return atlas_; }

  [[nodiscard]] const BlockSurfaceRegistry& surfaceRegistry() const { return surfaceRegistry_; }

  [[nodiscard]] const RenderGroupRegistry& renderGroupRegistry() const {
    return renderGroupRegistry_;
  }

  [[nodiscard]] const std::vector<BlockUvBuffer>& uvBuffers() const { return uvBuffers_; }

 private:
  void Build();

 private:
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
  Atlas atlas_;
  BlockSurfaceRegistry surfaceRegistry_;
  RenderGroupRegistry renderGroupRegistry_;
  std::vector<BlockUvBuffer> uvBuffers_;
  std::vector<BlockRenderInfo> blockInfos_;
};

}  // namespace gfx