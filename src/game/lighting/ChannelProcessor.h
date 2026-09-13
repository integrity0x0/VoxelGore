#pragma once

#include <glm/vec3.hpp>
#include <queue>

#include "../voxel/ChunkManager.h"
#include "../voxel/BlockManager.h"
#include "LightChunkStorage.h"
#include "ChannelDefinition.h"
#include "BlockLightCache.h"

namespace gm::lighting {

struct LightNode {
  glm::ivec3 pos;
  uint32_t strength;
};

class ChannelProcessor {
 public:
  ChannelProcessor(LightChunkStorage& storage,
                   const ChannelDefinition& definition, const BlockLightCache& blockLightCache, ChunkManager& chunkManager,
                   const BlockManager& blockManager)
      : storage_(&storage),
        definition_(&definition),
        chunkManager_(&chunkManager),
        blockManager_(&blockManager) {}

  void Spread(const glm::ivec3& pos, uint32_t strength) {
    if (strength == 0) return;
    spreadQueue_.emplace(pos, strength);
    storage_->SetLight(pos, strength);
  }

  void Spread(const glm::ivec3& pos) {
    Spread(pos, storage_->GetLight(pos));
  }

  void Remove(const glm::ivec3& pos) {
    uint8_t light = storage_->GetLight(pos);
    if (light == 0) {
      return;
    }

    removeQueue_.emplace(pos, light);
    storage_->SetLight(pos, 0);
  }

  void Update();

 private:
  void ProcessRemoveQueue();
  void ProcessSpreadQueue();
  LightChunkStorage* storage_;
  const ChannelDefinition* definition_;
  ChunkManager* chunkManager_;
  const BlockManager* blockManager_;
  std::queue<LightNode> removeQueue_;
  std::queue<LightNode> spreadQueue_;
};

}  // namespace gm::lighting
