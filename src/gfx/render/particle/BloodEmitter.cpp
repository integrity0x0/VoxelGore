#include "BloodEmitter.h"

#include <random>

namespace gfx {

namespace {

std::mt19937 gRandomEngine{std::random_device{}()};
std::uniform_real_distribution<float> gDist01(0.0f, 1.0f);

float RandFloat01() { return gDist01(gRandomEngine); }

float RandRange(float lo, float hi) { return lo + RandFloat01() * (hi - lo); }

glm::vec3 RandRange(const glm::vec3& lo, const glm::vec3& hi) {
  return {RandRange(lo.x, hi.x), RandRange(lo.y, hi.y), RandRange(lo.z, hi.z)};
}

}  // namespace


BloodEmitter::BloodEmitter(const Atlas& atlas, const gm::BlockManager& blockManager,
                           const gm::ChunkManager& chunkManager) 
    : PhysicalParticleEmitter(chunkManager, blockManager),
      blockManager_(&blockManager),
      chunkManager_(&chunkManager) {
  const AtlasRegion* reg = atlas.Find("blank");

  if (!reg) {
    throw std::runtime_error("BloodEmitter: Unable to find a blank texture");
  }

  blankRegion_ = reg->toUv();
}


void BloodEmitter::Spawn(const glm::vec3& pos, const glm::vec3& normal, float damage) {
  size_t count = static_cast<size_t>(damage * kCountPerDamage);
  for (size_t i = 0; i < count; ++i) {
    Particle particle;
    particle.maxLife = RandRange(kMinLife, kMaxLife);
    particle.acceleration = glm::vec3(0.0f, -25.3f, 0.0f);
    particle.atlasType = gfx::Particle::AtlasType::General;
    particle.bounceFactor = 0.0f;
    particle.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    particle.size = glm::vec2(0.15f);
    particle.uvMinMax = glm::vec4(blankRegion_.min, blankRegion_.max);
    particle.layer = static_cast<float>(blankRegion_.arrayLayer);
    particles_.emplace_back(particle);
  }
}

void BloodEmitter::Update(float dt) {
  UpdateParticles(dt);
}
}  // namespace gfx