#include "ExplosionEmitter.h"

#include <algorithm>
#include <random>

#include "../../common/texture/SpriteSheetParser.h"

namespace gfx {

const std::string ExplosionEmitter::kExplosionAnimation =
    core::kAssetsPrefix + "particles/explosion.json";

namespace {

std::mt19937 gRng{std::random_device{}()};
std::uniform_real_distribution<float> g01(0.0f, 1.0f);

float rand01() { return g01(gRng); }

float randRange(float lo, float hi) { return lo + rand01() * (hi - lo); }

glm::vec3 randInCube(float half) {
  return {
      randRange(-half, half),
      randRange(-half, half),
      randRange(-half, half),
  };
}

}  // namespace

ExplosionEmitter::ExplosionEmitter(Atlas& atlas) {
  variants_ = SpriteSheetParser::parseVariants(kExplosionAnimation, atlas);
}

void ExplosionEmitter::Spawn(const glm::vec3& position, float power) {
  ActiveEmitter em;
  em.pos = position;
  em.power = power;
  emitters_.push_back(em);
}

void ExplosionEmitter::emitBurst(const ActiveEmitter& em) {
  if (variants_.empty()) return;

  const float life01 = std::clamp(em.age / kEmitterLife, 0.0f, 1.0f);
  const float sizeScale = 1.0f - 0.6f * life01;
  const float baseSize = randRange(4.5f, 12.0f) * sizeScale * (em.power / 4.0f);

  for (int i = 0; i < kParticlesPerBurst; ++i) {
    size_t variantId = static_cast<size_t>(rand01() * variants_.size()) % variants_.size();
    const SpriteSheet& sheet = variants_[variantId];

    Particle p;
    p.renderLayer = RenderLayer::Cutout;
    p.atlasType = Particle::AtlasType::General;

    p.pos = em.pos + randInCube(kSpread);
    p.velocity = glm::vec3(0.0f);
    p.acceleration = glm::vec3(0.0f);

    p.rotation = randRange(0.0f, 6.2831853f);
    p.angularVelocity = 0.0f;

    p.size = glm::vec2(baseSize);
    p.color = glm::vec4(1.0f);

    p.life = kParticleLife;
    p.maxLife = kParticleLife;

    p.layer = 0.0f;
    p.variantId = static_cast<uint32_t>(variantId);
    p.ignoreLighting = true;

    const UvRegion& uv = sheet.frameAt(0);
    p.uvMinMax = glm::vec4(uv.min.x, uv.min.y, uv.max.x, uv.max.y);

    particles_.push_back(p);
  }
}

const SpriteSheet& ExplosionEmitter::pickVariant() const {
  const size_t idx = static_cast<size_t>(rand01() * variants_.size()) % variants_.size();

  return variants_[idx];
}

void ExplosionEmitter::Update(float dt) {
  for (auto& em : emitters_) {
    em.age += dt;

    while (em.age >= em.nextEmit && em.nextEmit < kEmitterLife) {
      emitBurst(em);
      em.nextEmit += kEmitInterval;
    }
  }

  emitters_.erase(std::remove_if(emitters_.begin(), emitters_.end(),
                                 [](const ActiveEmitter& e) { return e.age >= kEmitterLife; }),
                  emitters_.end());

  for (auto& p : particles_) {
    if (variants_.empty()) continue;

    const SpriteSheet& anim = variants_[p.variantId];

    const float age = p.maxLife - p.life;

    uint32_t frame = static_cast<uint32_t>((age / p.maxLife) * anim.frameCount());
    frame = std::min(frame, anim.frameCount() - 1u);

    const UvRegion& uv = anim.frameAt(frame);
    p.uvMinMax = glm::vec4(uv.min.x, uv.min.y, uv.max.x, uv.max.y);
  }

  UpdateParticles(dt);
}

}  // namespace gfx