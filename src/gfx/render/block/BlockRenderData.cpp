#include "BlockRenderData.h"

namespace gfx {

BlockRenderData::BlockRenderData(const gm::BlockManager& blockManager, const vkcore::Device& device,
                       vkcore::TransferContext& transferCtxt,
                       vkcore::MemoryAllocator& memoryAllocator, uint32_t framesInFlight)
    : blockManager_(&blockManager),
      transferCtxt_(&transferCtxt),
      atlas_(device, transferCtxt, memoryAllocator, glm::ivec2(2048), 6, 2, 128),
      surfaceRegistry_(atlas_) {
  uvBuffers_.reserve(framesInFlight);

  for (uint32_t i = 0; i < framesInFlight; ++i) {
    uvBuffers_.emplace_back(device, memoryAllocator, 512u);
  }

  blockInfos_.resize(blockManager.blockCount());

  Build();
}

void BlockRenderData::Build() {
  for (uint32_t blockId = 0; blockId < blockInfos_.size(); blockId++) {
    auto& info = blockInfos_[blockId];

    const gm::Block* block = blockManager_->block(blockId);

    if (!block) {
      continue;
    }

    for (uint32_t face = 0; face < kFaceCount; face++) {
      info.surfaces[face] = surfaceRegistry_.Resolve(block->getSurface(face), uvBuffers_);
    }

    info.renderGroup = renderGroupRegistry_.RegisterGroup(block->renderGroup());
  }
}

void BlockRenderData::Update(float dt, uint32_t currentFrameInFlight) {
  if (currentFrameInFlight >= uvBuffers_.size()) {
    return;
  }

  surfaceRegistry_.UpdateAnimations(dt, uvBuffers_[currentFrameInFlight]);
}

BlockSurfaceId BlockRenderData::surfaceId(uint32_t blockId, gm::Block::Face face) {
  if (blockId >= blockInfos_.size()) {
    return BlockSurfaceRegistry::kInvalidSurface;
  }

  const size_t faceIndex = static_cast<size_t>(face);

  if (faceIndex >= kFaceCount) {
    return BlockSurfaceRegistry::kInvalidSurface;
  }

  return blockInfos_[blockId].surfaces[faceIndex];
}

RenderGroupId BlockRenderData::renderGroupId(uint32_t blockId) {
  if (blockId >= blockInfos_.size()) {
    return RenderGroupRegistry::kInvalid;
  }

  return blockInfos_[blockId].renderGroup;
}

}  // namespace gfx