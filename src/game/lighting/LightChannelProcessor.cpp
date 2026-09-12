#include "LightChannelProcessor.h"

#include <array>

namespace gm {

namespace {
constexpr auto kNeighbourOffsets = std::to_array<glm::ivec3>({
    {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1},
});

bool PassesLight(const BlockManager* blockManager, uint32_t blockId) {
  const Block* block = blockManager->block(blockId);
  return block != nullptr && block->isPassingLight();
}
}  // namespace

void LightChannelProcessor::Update() {
  ProcessRemoveQueue();
  ProcessSpreadQueue();
}

void LightChannelProcessor::ProcessRemoveQueue() {
  while (!removeQueue_.empty()) {
    LightNode front = removeQueue_.front();
    removeQueue_.pop();

    for (const glm::ivec3& offset : kNeighbourOffsets) {
      glm::ivec3 neighborPos = front.pos + offset;

      std::optional<Voxel> voxel = chunkManager_->getVoxel(neighborPos);
      if (!voxel.has_value()) {
        continue;
      }

      uint32_t neighborLight = chunkManager_->getLight(neighborPos, channel_);

      if (neighborLight != 0 && neighborLight == front.strength - 1) {
        const Block* block = blockManager_->block(voxel->id);

        if (block) {
          glm::ivec3 emission = block->getEmission();
          uint32_t sourceLight = (channel_ != LightChannel::S) ? emission[static_cast<size_t>(channel_)] : 0;
          if (sourceLight != 0) {
            Spread(neighborPos, sourceLight);
          } else {
            Remove(neighborPos);
          }
        } else {
          Remove(neighborPos);
        }
      } else if (neighborLight >= front.strength) {
        Spread(neighborPos, neighborLight);
      }
    }

    chunkManager_->setLight(front.pos, channel_, 0);
  }
}

void LightChannelProcessor::ProcessSpreadQueue() {
  while (!spreadQueue_.empty()) {
    LightNode front = spreadQueue_.front();
    spreadQueue_.pop();

    chunkManager_->setLight(front.pos, channel_, front.strength);
    
    if (front.strength <= 1) continue;

    for (size_t i = 0; i < kNeighbourOffsets.size(); ++i) {
      glm::ivec3 neighborPos = front.pos + kNeighbourOffsets[i];

      std::optional<Voxel> voxel = chunkManager_->getVoxel(neighborPos);
      if (!voxel.has_value() || !PassesLight(blockManager_, voxel->id)) {
        continue;
      }

      uint32_t neighborLight = chunkManager_->getLight(neighborPos, channel_);
      
      if (neighborLight < front.strength - 1) {
        Spread(neighborPos, front.strength - 1);
      }

    }
  }
}

}  // namespace gm