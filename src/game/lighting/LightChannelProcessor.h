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
      : chunkManager_(&chunkManager), blockManager_(&blockManager), channel_(channel) {
    assert(channel < LightChannel::Count);
  }

  void Spread(const glm::ivec3& pos, uint32_t strength) {
    if (strength == 0) return;
    spreadQueue_.emplace(pos, strength);
    chunkManager_->setLight(pos, channel_, strength);
  }

  void Spread(const glm::ivec3& pos) {
    Spread(pos, chunkManager_->getLight(pos, channel_));
  }

  void Remove(const glm::ivec3& pos) {
    uint32_t light = chunkManager_->getLight(pos, channel_);
    if (light == 0) {
      return;
    }

    removeQueue_.emplace(pos, light);
    chunkManager_->setLight(pos, channel_, 0);
  }

  void Update();

 private:
  void ProcessRemoveQueue();
  void ProcessSpreadQueue();

  ChunkManager* chunkManager_;
  const BlockManager* blockManager_;
  LightChannel channel_;
  std::queue<LightNode> removeQueue_;
  std::queue<LightNode> spreadQueue_;
};

}  // namespace gm
