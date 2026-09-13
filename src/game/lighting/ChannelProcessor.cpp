#include "ChannelProcessor.h"

#include <array>

namespace gm::lighting {

namespace {
constexpr auto kNeighbourOffsets = std::to_array<glm::ivec3>({
    {1, 0, 0},
    {-1, 0, 0},
    {0, 1, 0},
    {0, -1, 0},
    {0, 0, 1},
    {0, 0, -1},
});

bool PassesLight(const BlockManager* blockManager, uint32_t blockId) {
  const Block* block = blockManager->block(blockId);
  return block != nullptr && block->isPassingLight();
}
}  // namespace

void ChannelProcessor::Update() {
  ProcessRemoveQueue();
  ProcessSpreadQueue();
}

void ChannelProcessor::ProcessRemoveQueue() {
  while (!removeQueue_.empty()) {
    LightNode front = removeQueue_.front();
    removeQueue_.pop();

    for (const glm::ivec3& offset : kNeighbourOffsets) {
      const glm::ivec3 neighborPos = front.pos + offset;

      const std::optional<Voxel> voxel = chunkManager_->getVoxel(neighborPos);
      if (!voxel.has_value()) {
        continue;
      }

      const uint8_t neighborLight = storage_->GetLight(neighborPos, id_);

      if (neighborLight != 0 && neighborLight == front.strength - 1) {
        const auto* light = blockCache_->Require(voxel->id);

        if (light && light->id == id_ && light->strength != 0) {
          Spread(neighborPos, light->strength);
        } else {
          Remove(neighborPos);
        }
      } else if (neighborLight >= front.strength) {
        Spread(neighborPos, neighborLight);
      }
    }

    storage_->SetLight(front.pos, id_, 0);
  }
}

void ChannelProcessor::ProcessSpreadQueue() {
  while (!spreadQueue_.empty()) {
    LightNode front = spreadQueue_.front();
    spreadQueue_.pop();

    storage_->SetLight(front.pos, id_, front.strength);

    if (front.strength <= 1) {
      continue;
    }

    for (const glm::ivec3& offset : kNeighbourOffsets) {
      const glm::ivec3 neighborPos = front.pos + offset;

      const std::optional<Voxel> voxel = chunkManager_->getVoxel(neighborPos);
      if (!voxel.has_value() || !PassesLight(blockManager_, voxel->id)) {
        continue;
      }

      const uint8_t neighborLight = storage_->GetLight(neighborPos, id_);

      if (neighborLight < front.strength - 1) {
        Spread(neighborPos, front.strength - 1);
      }
    }
  }
}

}  // namespace gm::lighting