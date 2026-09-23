#pragma once

#include "ParticleEmitter.h"
#include "../../../game/voxel/BlockManager.h"
#include "../../../game/voxel/ChunkManager.h"

namespace gfx {
class PhysicalParticleEmitter : public ParticleEmitter {
 public:
  PhysicalParticleEmitter(const gm::ChunkManager& chunkManager,
                          const gm::BlockManager& blockManager);
 protected:
  [[nodiscard]] bool IsObstacle(const glm::ivec3& pos);
  void ResolveCollisions(Particle& p, float dt);

 private:
  const gm::ChunkManager* chunkManager_;
  const gm::BlockManager* blockManager_;
};
}  // namespace gfx