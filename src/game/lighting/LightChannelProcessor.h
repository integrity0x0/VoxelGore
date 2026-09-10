#pragma once

#include <glm/vec3.hpp>
#include <queue>

#include "../voxel/BlockManager.h"
#include "../voxel/ChunkManager.h"
#include "LightMap.h"

namespace gm {

struct LightNode {
  glm::ivec3 pos;
  uint32_t strength;
};

class LightChannelProcessor {
 public:
  LightChannelProcessor(ChunkManager& chunkManager, const BlockManager& blockManager,
                        LightChannel channel)
      : chunkManager(&chunkManager), blockManager(&blockManager), channel(channel) {}

  void spread(const glm::ivec3& pos, uint32_t strength) {
    if (!strength) return;
    spreadQueue.push({pos, strength});
    chunkManager->setLight(pos, channel, static_cast<uint8_t>(strength));
  }

  void remove(const glm::ivec3& pos) {
    removeQueue.push({pos, static_cast<uint32_t>(chunkManager->getLight(pos, channel))});
    chunkManager->setLight(pos, channel, 0);
  }

  void Update();

 private:
  void processRemoveQueue();
  void processSpreadQueue();

  ChunkManager* chunkManager;
  const BlockManager* blockManager;
  LightChannel channel;
  std::queue<LightNode> removeQueue;
  std::queue<LightNode> spreadQueue;
};

}  // namespace gm
