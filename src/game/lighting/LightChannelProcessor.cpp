#include "LightChannelProcessor.h"

#include <optional>

#include "../voxel/ChunkManager.h"

namespace gm {

namespace {
constexpr glm::ivec3 kNeighbourOffsets[6] = {
    {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1},
};

bool passesLight(const BlockManager* blockManager, uint32_t blockId) {
  const Block* block = blockManager->block(blockId);
  return block != nullptr && block->isPassingLight();
}
}  // namespace

void LightChannelProcessor::Update() {
  processRemoveQueue();
  processSpreadQueue();
}

void LightChannelProcessor::processRemoveQueue() {
  std::vector<glm::ivec3> affected;
  while (!removeQueue.empty()) {
    LightNode front = removeQueue.front();
    removeQueue.pop();

    for (size_t i = 0; i < 6; ++i) {
      glm::ivec3 neighbourPos = front.pos + kNeighbourOffsets[i];
      std::optional<Voxel> voxel = chunkManager->getVoxel(neighbourPos);

      if (!voxel.has_value() || !passesLight(blockManager, voxel->id)) continue;

      uint32_t neighbourLight = chunkManager->getLight(neighbourPos, channel);

      if (neighbourLight != 0 && neighbourLight < front.strength) {
        remove(neighbourPos);
      } else if (neighbourLight >= front.strength) {
        spread(neighbourPos, neighbourLight);
      }
    }
  }

  removeQueue = {};
}

void LightChannelProcessor::processSpreadQueue() {
  while (!spreadQueue.empty()) {
    LightNode front = spreadQueue.front();
    spreadQueue.pop();

    if (front.strength <= 1) continue;

    uint32_t nextStrength = front.strength - 1;

    for (size_t i = 0; i < 6; ++i) {
      glm::ivec3 neighbourPos = front.pos + kNeighbourOffsets[i];
      std::optional<Voxel> voxel = chunkManager->getVoxel(neighbourPos);

      if (!voxel.has_value()) continue;

      if (!passesLight(blockManager, voxel->id)) continue;
      uint32_t neighbourLight = chunkManager->getLight(neighbourPos, channel);

      if (neighbourLight < nextStrength) {
        chunkManager->setLight(neighbourPos, channel, nextStrength);
        spread(neighbourPos, nextStrength);
      }
    }
  }

  spreadQueue = {};
}

}  // namespace gm
