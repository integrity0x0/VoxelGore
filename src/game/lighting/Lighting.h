#pragma once

#include <memory>
#include <vector>

#include "../voxel/BlockManager.h"
#include "BlockLightCache.h"
#include "ChannelRegistry.h"
#include "LightChannel.h"
#include "LightChunkStorage.h"
#include "../Enviroment.h"

namespace gm {

class Lighting {
 public:
  static inline std::string kSunName = "sun";
  static inline ChannelId kSunId = 0;

  Lighting(ChunkManager& chunkManager, const BlockManager& blockManager)
      : chunkManager_(&chunkManager),
        blockManager_(&blockManager),
        registry_(ChannelDefinition(kSunName, {}/*unused*/)),
        blockCache_(blockManager, registry_),
        sun_(*registry_.GetByName(kSunName), storage_, kSunId, blockCache_, *chunkManager_,
             *blockManager_) {}

  void LightUp();
  void OnVoxelSetted(const glm::ivec3& pos, const Voxel& voxel);

  [[nodiscard]] glm::vec4 GetColor(const glm::ivec3& pos) const;

  [[nodiscard]] const LightChunkStorage& storage() const { return storage_; };

  [[nodiscard]] const std::vector<std::unique_ptr<LightChannel>>& channels() const {
    return channels_;
  }

 private:
  [[nodiscard]] LightChannel& RequireChannel(ChannelId id);

 private:
  ChunkManager* chunkManager_;
  const BlockManager* blockManager_;

  ChannelRegistry registry_;
  BlockLightCache blockCache_;

  LightChunkStorage storage_;

  LightChannel sun_;
  std::vector<std::unique_ptr<LightChannel>> channels_;

  mutable glm::ivec3 lastCachedPos_ = glm::ivec3(0);
  mutable const LightChunkStorage::ChunkData* lastCachedChunks_ = nullptr;
};

}  // namespace gm