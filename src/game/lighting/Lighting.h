#pragma once

#include <memory>
#include <vector>

#include "../voxel/BlockManager.h"
#include "BlockLightCache.h"
#include "ChannelRegistry.h"
#include "LightChannel.h"
#include "LightChunkStorage.h"

namespace gm::lighting {

class Lighting {
 public:
  static inline std::string kSunName = "sun";
  static inline ChannelId kSunId = 0;

  Lighting(ChunkManager& chunkManager, const BlockManager& blockManager)
      : chunkManager_(&chunkManager),
        blockManager_(&blockManager),
        registry_(ChannelDefinition(kSunName, glm::vec3(1.0f))),
        blockCache_(blockManager, registry_),
        sun_(*registry_.GetByName(kSunName), storage_, kSunId, blockCache_, *chunkManager_,
             *blockManager_) {}

  void LightUp();
  void OnVoxelSetted(const glm::ivec3& pos, const Voxel& voxel);

  [[nodiscard]] glm::vec4 GetColor(const glm::ivec3& pos) const;

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
};

}  // namespace gm::lighting