#include "BloodEmitter.h"

#include <cmath>
#include <random>
#include <numbers>

namespace gfx {

namespace {

std::mt19937 gRandomEngine{std::random_device{}()};
std::uniform_real_distribution<float> gDist01(0.0f, 1.0f);

float RandFloat01() { return gDist01(gRandomEngine); }

float RandRange(float lo, float hi) { return lo + RandFloat01() * (hi - lo); }

glm::vec3 RandRange(const glm::vec3& lo, const glm::vec3& hi) {
  return {RandRange(lo.x, hi.x), RandRange(lo.y, hi.y), RandRange(lo.z, hi.z)};
}

glm::vec3 RandomConeDirection(const glm::vec3& normal, float minAngle, float maxAngle) {
  glm::vec3 axis = glm::normalize(normal);
  if (glm::dot(axis, axis) < 0.0001f) axis = glm::vec3(0.0f, 1.0f, 0.0f);

  const glm::vec3 reference =
      std::abs(axis.y) < 0.999f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);

  const glm::vec3 tangent = glm::normalize(glm::cross(reference, axis));
  const glm::vec3 bitangent = glm::cross(axis, tangent);

  const float minCos = std::cos(minAngle);
  const float maxCos = std::cos(maxAngle);
  const float cosTheta = RandRange(maxCos, minCos);
  const float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
  const float phi = RandRange(0.0f, 2.0f * std::numbers::pi_v<float>);

  return axis * cosTheta + tangent * (std::cos(phi) * sinTheta) +
         bitangent * (std::sin(phi) * sinTheta);
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
  const size_t count = static_cast<size_t>(damage * kCountPerDamage);
  const float minAngle = glm::radians(kMinAngle);
  const float maxAngle = glm::radians(kMaxAngle);

  for (size_t i = 0; i < count; ++i) {
    Particle particle;

    const glm::vec3 startSpread(RandRange(-kSpawnSpread, kSpawnSpread),
                                RandRange(-kSpawnSpread, kSpawnSpread),
                                RandRange(-kSpawnSpread, kSpawnSpread));

    const glm::vec3 direction = RandomConeDirection(normal, minAngle, maxAngle);
    const float life = RandRange(kMinLife, kMaxLife);
    const float size = RandRange(kMinSize, kMaxSize);

    particle.pos = pos + normal * kSpawnOffset + startSpread;
    particle.velocity = direction * RandRange(kMinSpeed, kMaxSpeed);
    particle.maxLife = life;
    particle.life = life;
    particle.acceleration = glm::vec3(0.0f, -25.3f, 0.0f);
    particle.atlasType = Particle::AtlasType::General;
    particle.bounceFactor = 0.0f;
    particle.color = glm::vec4(0.45f, 0.01f, 0.015f, 1.0f);
    particle.size = glm::vec2(size);
    particle.uvMinMax = glm::vec4(blankRegion_.min, blankRegion_.max);
    particle.layer = static_cast<float>(blankRegion_.arrayLayer);
    particle.renderLayer = RenderLayer::Solid;
    particle.ignoreLighting = false;
    particle.collision = true;
    particles_.emplace_back(particle);
  }
}

void BloodEmitter::Update(float dt) { UpdateParticles(dt); }

}  // namespace gfx