#pragma once

#include "../../../game/voxel/ChunkManager.h"
#include "../../../game/lighting/Lighting.h"
#include "../billboard/BillboardRenderBucket.h"
#include "BlockDebrisEmitter.h"
#include "ExplosionEmitter.h"
#include "BloodEmitter.h"
#include "../../../game/Enviroment.h"

namespace gfx {

class ParticleEngine {
 public:
  ParticleEngine(Atlas& generalAtlas, BlockRenderData& renderData,
                 const gm::BlockManager& blockManager, const gm::ChunkManager& chunkManager,
                 const gm::Lighting& lighting, const gm::Enviroment& enviroment,
                 BillboardRenderBucket& generalBucket, BillboardRenderBucket& blockBucket);

  void SpawnExplosion(const glm::vec3& pos, float power = 4.0f);

  void SpawnBlockDebris(uint32_t blockId, const glm::vec3& pos);

  void SpawnBlood(const glm::vec3& pos, const glm::vec3& normal, float damage);

  void Update(float dt, uint32_t currentFrame);

 private:
  Atlas* generalAtlas_;
  const gm::ChunkManager* chunkManager_;
  const gm::Lighting* lighting_;
  const gm::Enviroment* enviroment_;
  BlockRenderData* blockRenderData_;
  BillboardRenderBucket* generalBucket_;
  BillboardRenderBucket* blockBucket_;

  BlockDebrisEmitter blockDebrisEmitter_;
  ExplosionEmitter explosionEmitter_;
  BloodEmitter bloodEmitter_;

  BillboardRenderBucket& BucketFor(const Particle& particle);

  void WriteParticles(std::span<const Particle> particles, uint32_t currentFrame);
};

}  // namespace gfx