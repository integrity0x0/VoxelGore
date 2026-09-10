#include "RenderData.h"

namespace gfx::block {

RenderData::RenderData(const gm::BlockManager& blockManager, const vkcore::Device& device,
                       vkcore::TransferContext& transferCtxt,
                       vkcore::MemoryAllocator& memoryAllocator, uint32_t framesInFlight)
    : blockManager_(&blockManager),
      transferCtxt_(&transferCtxt),
      atlas_(device, transferCtxt, memoryAllocator, glm::ivec2(2048), 6u, 2u),
      surfaceRegistry_(atlas_) {
  uvBuffers_.reserve(framesInFlight);

  for (uint32_t i = 0; i < framesInFlight; ++i) {
    uvBuffers_.emplace_back(device, memoryAllocator, 512u);
  }

  blockInfos_.resize(blockManager.blockCount());

  build();
}

void RenderData::build() {
  for (uint32_t blockId = 0; blockId < blockInfos_.size(); blockId++) {
    auto& info = blockInfos_[blockId];

    const gm::Block* block = blockManager_->block(blockId);

    if (!block) {
      continue;
    }

    for (uint32_t face = 0; face < kFaceCount; face++) {
      info.surfaces[face] = surfaceRegistry_.resolve(block->getSurface(face), uvBuffers_);
    }

    info.renderGroup = renderGroupRegistry_.registerGroup(block->renderGroup());
  }
}

void RenderData::Update(float dt, uint32_t currentFrameInFlight) {
  if (currentFrameInFlight >= uvBuffers_.size()) {
    return;
  }

  surfaceRegistry_.updateAnimations(dt, uvBuffers_[currentFrameInFlight]);
}

SurfaceId RenderData::surfaceId(uint32_t blockId, gm::Block::Face face) {
  if (blockId >= blockInfos_.size()) {
    return SurfaceRegistry::kInvalidSurface;
  }

  const size_t faceIndex = static_cast<size_t>(face);

  if (faceIndex >= kFaceCount) {
    return SurfaceRegistry::kInvalidSurface;
  }

  return blockInfos_[blockId].surfaces[faceIndex];
}

RenderGroupId RenderData::renderGroupId(uint32_t blockId) {
  if (blockId >= blockInfos_.size()) {
    return RenderGroupRegistry::kInvalid;
  }

  return blockInfos_[blockId].renderGroup;
}

}  // namespace gfx::block