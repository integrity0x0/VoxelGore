#pragma once

#include "ParticleEmitter.h"
#include "../../../game/voxel/ChunkManager.h"
#include "../../../game/voxel/BlockManager.h"

namespace gfx {
class BloodEmitter : public ParticleEmitter {
 public:
  BloodEmitter(const gm::BlockManager& blockManager, const gm::ChunkManager& chunkManager);
 private:
  const gm::BlockManager* blockManager_;
  const gm::ChunkManager* chunkManager_;
};
}  // namespace gfx
