#pragma once

#include "../../../game/voxel/ChunkManager.h"
#include "../billboard/BillboardRenderBucket.h"
#include "BlockDebrisEmitter.h"
#include "ExplosionEmitter.h"

namespace gfx {

class ParticleEngine {
 public:
  ParticleEngine(Atlas& generalAtlas, block::RenderData& renderData,
                 const gm::BlockManager& blockManager, const gm::ChunkManager& chunkManager,
                 BillboardRenderBucket& generalBucket, BillboardRenderBucket& blockBucket);

  void SpawnExplosion(const glm::vec3& position, float power = 4.0f);

  void SpawnBlockDebris(uint32_t blockId, const glm::vec3& position);

  void Update(float dt, uint32_t currentFrame);

 private:
  Atlas* generalAtlas_;
  const gm::ChunkManager* chunkManager_;
  block::RenderData* blockRenderData_;
  BillboardRenderBucket* generalBucket_;
  BillboardRenderBucket* blockBucket_;

  BlockDebrisEmitter blockDebrisEmitter_;
  ExplosionEmitter explosionEmitter_;

  BillboardRenderBucket& BucketFor(const Particle& particle);

  void WriteParticles(std::span<const Particle> particles, uint32_t currentFrame);
};

}  // namespace gfx