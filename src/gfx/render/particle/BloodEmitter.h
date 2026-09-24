#pragma once

#include "../../../game/voxel/BlockManager.h"
#include "../../../game/voxel/ChunkManager.h"
#include "../../common/texture/Atlas.h"
#include "PhysicalParticleEmitter.h"

namespace gfx {

class BloodEmitter : public PhysicalParticleEmitter {
 public:
  BloodEmitter(const Atlas& atlas, const gm::BlockManager& blockManager,
               const gm::ChunkManager& chunkManager);

  void Spawn(const glm::vec3& pos, const glm::vec3& normal, float damage);
  void Update(float dt) override;

 private:
  static constexpr float kMinLife = 0.3f;
  static constexpr float kMaxLife = 1.5f;
  static constexpr float kMinSpeed = 2.0f;
  static constexpr float kMaxSpeed = 6.0f;
  static constexpr float kMinAngle = 5.0f;
  static constexpr float kMaxAngle = 65.0f;
  static constexpr float kSpawnSpread = 0.05f;
  static constexpr float kSpawnOffset = 0.02f;
  static constexpr float kMinSize = 0.10f;
  static constexpr float kMaxSize = 0.20f;
  static constexpr float kCountPerDamage = 1.8f;

  const gm::BlockManager* blockManager_;
  const gm::ChunkManager* chunkManager_;
  UvRegion blankRegion_ = {};
};

}  // namespace gfx