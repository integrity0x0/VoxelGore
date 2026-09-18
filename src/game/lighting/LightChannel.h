#pragma once

#include "LightChunkStorage.h"
#include "ChannelDefinition.h"
#include "ChannelProcessor.h"
#include "../voxel/ChunkManager.h"
#include "../voxel/BlockManager.h"
#include "BlockLightCache.h"

namespace gm {
class LightChannel {
 public:
  LightChannel(const ChannelDefinition& definition, LightChunkStorage& storage, ChannelId id,
               BlockLightCache& blockCache, ChunkManager& chunkManager,
               const BlockManager& blockManager)
      : definition_(&definition),
        storage_(&storage),
        id_(id),
        processor_(storage, definition, id, blockCache, chunkManager, blockManager) {}

  void Spread(const glm::ivec3& pos, uint32_t strength) { processor_.Spread(pos, strength); }

  void Spread(const glm::ivec3& pos) { processor_.Spread(pos); }

  void Remove(const glm::ivec3& pos) { processor_.Remove(pos); }

  void Update() { processor_.Update(); }

  [[nodiscard]] uint8_t GetLight(const glm::ivec3& pos) const { return storage_->GetLight(pos, id_); }

  [[nodiscard]] const ChannelDefinition& definition() const { return *definition_; }

 private:
  const ChannelDefinition* definition_;
  ChannelId id_;
  LightChunkStorage* storage_;
  ChannelProcessor processor_;
};

}  // namespace gm