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
  static glm::vec3 ambientColor = enviroment_->GetColor();

  for (const Particle& particle : particles) {
    BillboardInstance instance;

    instance.pos = particle.pos;
    instance.rotation = particle.rotation;
    instance.size = particle.size;
    instance.uvMinMax = particle.uvMinMax;
    instance.layer = particle.layer;

    glm::vec4 lightColor =
        particle.ignoreLighting ? glm::vec4(1.0f) : lighting_->GetColor(glm::ivec3(particle.pos));

    glm::vec3 color = glm::vec3(lightColor) + lightColor.a * ambientColor;
    color = glm::clamp(color, 0.0f, 1.0f);

    instance.color = glm::vec4(color, 1.0f);

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