#include "ParticleEngine.h"

namespace gfx {

ParticleEngine::ParticleEngine(Atlas& generalAtlas, BlockRenderData& blockRenderData,
                               const gm::BlockManager& blockManager,
                               const gm::ChunkManager& chunkManager,
                               const gm::Lighting& lighting,
                               const gm::Enviroment& enviroment,
                               BillboardRenderBucket& generalBucket,
                               BillboardRenderBucket& blockBucket)
    : generalAtlas_(&generalAtlas),
      chunkManager_(&chunkManager),
      lighting_(&lighting),
      enviroment_(&enviroment),
      blockRenderData_(&blockRenderData),
      generalBucket_(&generalBucket),
      blockBucket_(&blockBucket),
      blockDebrisEmitter_(blockRenderData, chunkManager, blockManager),
      explosionEmitter_(generalAtlas),
      bloodEmitter_(generalAtlas, blockManager, chunkManager) {}

void ParticleEngine::SpawnBlockDebris(uint32_t blockId, const glm::vec3& pos) {
  blockDebrisEmitter_.Spawn(blockId, pos);
}

void ParticleEngine::SpawnExplosion(const glm::vec3& pos, float power) {
  explosionEmitter_.Spawn(pos, power);
}

void ParticleEngine::SpawnBlood(const glm::vec3& pos, const glm::vec3& normal, float damage) {
  bloodEmitter_.Spawn(pos, normal, damage);
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

    glm::vec3 color;

    if (particle.ignoreLighting) {
      color = glm::vec3(1.0f);
    } else {
      const glm::vec4 lightColor = lighting_->GetColor(glm::ivec3(particle.pos));
      const glm::vec3 ambientColor = enviroment_->GetColor();

      color = glm::vec3(lightColor) + lightColor.a * ambientColor;
      color = glm::clamp(color, 0.0f, 1.0f);
    }

    instance.color = glm::vec4(color, 1.0f) * particle.color;

    BucketFor(particle).Submit(instance, particle.renderLayer, currentFrame);
  }
}

void ParticleEngine::Update(float dt, uint32_t currentFrame) {
  blockDebrisEmitter_.Update(dt);
  explosionEmitter_.Update(dt);
  bloodEmitter_.Update(dt);
  WriteParticles(blockDebrisEmitter_.particles(), currentFrame);
  WriteParticles(explosionEmitter_.particles(), currentFrame);
  WriteParticles(bloodEmitter_.particles(), currentFrame);
}

}  // namespace gfx