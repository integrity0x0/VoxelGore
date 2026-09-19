#pragma once

#include <glm/vec3.hpp>
#include <queue>

#include "../voxel/BlockManager.h"
#include "../voxel/ChunkManager.h"
#include "BlockLightCache.h"
#include "ChannelDefinition.h"
#include "LightChunkStorage.h"

namespace gm {

struct LightNode {
  glm::ivec3 pos;
  uint32_t strength;
  LightChunk* chunk;
};

class ChannelProcessor {
 public:
  ChannelProcessor(LightChunkStorage& storage, const ChannelDefinition& definition, ChannelId id,
                   BlockLightCache& blockCache, ChunkManager& chunkManager,
                   const BlockManager& blockManager)
      : storage_(&storage),
        definition_(&definition),
        id_(id),
        blockCache_(&blockCache),
        chunkManager_(&chunkManager),
        blockManager_(&blockManager) {}

  void Spread(const glm::ivec3& pos, uint32_t strength);

  void Spread(const glm::ivec3& pos);

  void Remove(const glm::ivec3& pos);

  void Update();

 private:
  void Spread(const glm::ivec3& pos, uint32_t strength, LightChunk* chunk);

  void Remove(const glm::ivec3& pos, LightChunk* chunk);

  LightChunk* GetNeighborChunk(LightChunk* chunk, const glm::ivec3& neighborPos,
                               glm::ivec3& outLocalPos);

  void ProcessRemoveQueue();

  void ProcessSpreadQueue();

  LightChunkStorage* storage_;
  const ChannelDefinition* definition_;
  ChannelId id_;
  BlockLightCache* blockCache_;
  ChunkManager* chunkManager_;
  const BlockManager* blockManager_;

  std::queue<LightNode> removeQueue_;
  std::queue<LightNode> spreadQueue_;
};

}  // namespace gm