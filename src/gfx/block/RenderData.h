#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "../../game/voxel/BlockManager.h"
#include "../../vkcore/devices/Device.h"
#include "../texture/Atlas.h"
#include "RenderGroupRegistry.h"
#include "SurfaceRegistry.h"
#include "UvBuffer.h"

namespace gfx::block {
class RenderData {
 public:
  static constexpr uint32_t kFaceCount = 6;

  RenderData(const gm::BlockManager& blockManager, const vkcore::Device& device,
             vkcore::TransferContext& transferCtxt, vkcore::MemoryAllocator& memoryAllocator,
             uint32_t framesInFlight);

  void Update(float dt, uint32_t currentFrameInFlight);

  [[nodiscard]] const UvRegion& ExtractRegion(uint32_t blockId, gm::Block::Face face) {
    return surfaceRegistry_.ExtractRegion(
        blockInfos_[blockId].surfaces[static_cast<uint32_t>(face)]);
  }

  [[nodiscard]] SurfaceId surfaceId(uint32_t blockId, gm::Block::Face face);

  [[nodiscard]] RenderGroupId renderGroupId(uint32_t blockId);

  [[nodiscard]] const Atlas& atlas() const { return atlas_; }

  [[nodiscard]] Atlas& atlas() { return atlas_; }

  [[nodiscard]] const SurfaceRegistry& surfaceRegistry() const { return surfaceRegistry_; }

  [[nodiscard]] const RenderGroupRegistry& renderGroupRegistry() const {
    return renderGroupRegistry_;
  }

  [[nodiscard]] const std::vector<UvBuffer>& uvBuffers() const { return uvBuffers_; }

 private:
  void Build();

 private:
  struct BlockRenderInfo {
    static constexpr uint32_t kFaceCount = 6u;

    std::array<SurfaceId, kFaceCount> surfaces;
    RenderGroupId renderGroup;

    BlockRenderInfo() : renderGroup(RenderGroupRegistry::kInvalid) {
      surfaces.fill(SurfaceRegistry::kInvalidSurface);
    }
  };

  const gm::BlockManager* blockManager_;
  vkcore::TransferContext* transferCtxt_;
  Atlas atlas_;
  SurfaceRegistry surfaceRegistry_;
  RenderGroupRegistry renderGroupRegistry_;
  std::vector<UvBuffer> uvBuffers_;
  std::vector<BlockRenderInfo> blockInfos_;
};

}  // namespace gfx::block