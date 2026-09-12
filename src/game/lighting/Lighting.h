#pragma once

#include "../voxel/BlockManager.h"
#include "LightChannelProcessor.h"

namespace gm {

class Lighting {
 public:
  explicit Lighting(ChunkManager& chunkManager, const BlockManager& blockManager)
      : chunkManager(&chunkManager),
        blockManager(&blockManager),
        s(chunkManager, blockManager, LightChannel::S),
        r(chunkManager, blockManager, LightChannel::R),
        g(chunkManager, blockManager, LightChannel::G),
        b(chunkManager, blockManager, LightChannel::B) {}

  void LightUp();
  void OnVoxelSetted(const glm::ivec3& pos, const Voxel& voxel);

 private:
  ChunkManager* chunkManager;
  const BlockManager* blockManager;

  LightChannelProcessor s;
  LightChannelProcessor r;
  LightChannelProcessor g;
  LightChannelProcessor b;
};

}  // namespace gm