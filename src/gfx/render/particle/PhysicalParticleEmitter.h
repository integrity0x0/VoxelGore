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
  [[nodiscard]] bool IsObstacle(const glm::ivec3& pos) const;
  void ResolveCollision(Particle& p, float dt);
  void UpdateParticles(float dt) override;
 protected:
  const gm::ChunkManager* chunkManager_;
  const gm::BlockManager* blockManager_;
};
}  // namespace gfx