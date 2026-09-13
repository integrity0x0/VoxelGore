#include "ParticleEngine.h"

namespace gfx {

ParticleEngine::ParticleEngine(Atlas& generalAtlas, block::RenderData& blockRenderData,
                               const gm::BlockManager& blockManager,
                               const gm::ChunkManager& chunkManager,
                               BillboardRenderBucket& generalBucket,
                               BillboardRenderBucket& blockBucket)
    : generalAtlas_(&generalAtlas),
      chunkManager_(&chunkManager),
      blockRenderData_(&blockRenderData),
      generalBucket_(&generalBucket),
      blockBucket_(&blockBucket),
      blockDebrisEmitter_(blockRenderData, blockManager, chunkManager),
      explosionEmitter_(generalAtlas) {}

void ParticleEngine::SpawnBlockDebris(uint32_t blockId, const glm::vec3& position) {
  blockDebrisEmitter_.Spawn(blockId, position);
}

void ParticleEngine::SpawnExplosion(const glm::vec3& position, float power) {
  explosionEmitter_.Spawn(position, power);
}

BillboardRenderBucket& ParticleEngine::BucketFor(const Particle& particle) {
  switch (particle.atlasType) {
    case Particle::AtlasType::Terrain:
      return *blockBucket_;
    default:
    case Particle::AtlasType::General:
      return *generalBucket_;
  }
}

void ParticleEngine::WriteParticles(std::span<const Particle> particles, uint32_t currentFrame) {
  for (const Particle& particle : particles) {
    BillboardInstance instance;

    instance.pos = particle.pos;
    instance.rotation = particle.rotation;
    instance.size = particle.size;
    instance.uvMinMax = particle.uvMinMax;
    instance.layer = particle.layer;
    //instance.color =
       // particle.ignoreLighting
          //  ? particle.color
           // : particle.color *
            //      glm::vec4(chunkManager_->getLightColor(glm::ivec3(particle.pos)), 1.0f);
    BucketFor(particle).Submit(instance, particle.renderLayer, currentFrame);
  }
}

void ParticleEngine::Update(float dt, uint32_t currentFrame) {
  blockDebrisEmitter_.Update(dt);
  explosionEmitter_.Update(dt);

  WriteParticles(blockDebrisEmitter_.particles(), currentFrame);
  WriteParticles(explosionEmitter_.particles(), currentFrame);
}

}  // namespace gfx