#pragma once

#include "PhysicalParticleEmitter.h"
#include "../../../game/voxel/ChunkManager.h"
#include "../../../game/voxel/BlockManager.h"
#include "../../common/texture/Atlas.h"

namespace gfx {
class BloodEmitter : public PhysicalParticleEmitter {
 public:
  BloodEmitter(const Atlas& atlas, const gm::BlockManager& blockManager,
               const gm::ChunkManager& chunkManager);

  void Spawn(const glm::vec3& pos, const glm::vec3& normal, float damage);

  void Update(float dt) override;
 private:
  static constexpr float kMinLife = 0.25f;
  static constexpr float kMaxLife = 1.5f;
  static constexpr glm::vec3 kVelocityMin = {-2.0f, 1.0f, -2.0f};
  static constexpr glm::vec3 kVelocityMax = {2.0f, 4.0f, 2.0f};
  static constexpr float kCountPerDamage = 0.15f;
  const gm::BlockManager* blockManager_;
  const gm::ChunkManager* chunkManager_;
  UvRegion blankRegion_ = {};
};
}  // namespace gfx