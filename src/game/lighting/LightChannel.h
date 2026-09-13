#pragma once

#include "LightChunkStorage.h"
#include "ChannelDefinition.h"
#include "ChannelProcessor.h"
#include "../voxel/ChunkManager.h"
#include "../voxel/BlockManager.h"

namespace gm::lighting {
class LightChannel {
 public:
  LightChannel(const ChannelDefinition& definition, ChunkManager& chunkManager,
               const BlockManager& blockManager)
      : definition_(&definition), storage_(), processor_(storage_, chunkManager, blockManager) {}

  void Spread(const glm::ivec3& pos, uint32_t strength) { processor_.Spread(pos, strength); }

  void Spread(const glm::ivec3& pos) { processor_.Spread(pos); }

  void Remove(const glm::ivec3& pos) { processor_.Remove(pos); }

  void Update() { processor_.Update(); }

  [[nodiscard]] uint8_t GetLight(const glm::ivec3& pos) const { return storage_.GetLight(pos); }

  [[nodiscard]] const ChannelDefinition& definition() const { return *definition_; }

 private:
  const ChannelDefinition* definition_;
  LightChunkStorage storage_;
  ChannelProcessor processor_;
};

}  // namespace gm::lighting