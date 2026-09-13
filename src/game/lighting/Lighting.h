#pragma once

#include <memory>
#include <vector>

#include "../voxel/BlockManager.h"
#include "BlockLightCache.h"
#include "ChannelRegistry.h"
#include "LightChannel.h"

namespace gm::lighting {

class Lighting {
 public:
  static inline std::string kSunId = "sun";

  Lighting(ChunkManager& chunkManager, const BlockManager& blockManager)
      : chunkManager_(&chunkManager),
        blockManager_(&blockManager),
        registry_(ChannelDefinition(kSunId, {})),
        sun_(*registry_.Get(kSunId), *chunkManager_, *blockManager_) {}

  void LightUp();
  void OnVoxelSetted(const glm::ivec3& pos, const Voxel& voxel);

  glm::vec3 GetColor(const glm::ivec3& pos) { 
    glm::vec3 sunColor = sun_.definition().color *
      static_cast<float>(sun_.GetLight(pos)) / 15.0f;
    return sunColor;
  }

 private:
  [[nodiscard]] const BlockLightData* RequireBlockLight(uint32_t id);
  [[nodiscard]] LightChannel& RequireChannel(ChannelId id);
 private:
  ChunkManager* chunkManager_;
  const BlockManager* blockManager_;

  ChannelRegistry registry_;
  BlockLightCache blockCache_;

  LightChannel sun_;

  std::vector<std::unique_ptr<LightChannel>> channels_;
};

}  // namespace gm::lighting