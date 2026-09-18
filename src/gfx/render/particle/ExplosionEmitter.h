#pragma once

#include <string>
#include <vector>

#include "../../../core/PathPrefixes.h"
#include "../../common/texture/SpriteSheet.h"
#include "ParticleEmitter.h"

namespace gfx {

class ExplosionEmitter final : public ParticleEmitter {
 public:
  ExplosionEmitter(Atlas& atlas);

  void Spawn(const glm::vec3& position, float power = 4.0f);

  void Update(float dt) override;

 private:
  static constexpr float kEmitterLife = 0.4f;
  static constexpr float kParticleLife = 0.4f;
  static constexpr float kEmitInterval = 0.05f;

  static constexpr int kParticlesPerBurst = 1;

  static constexpr float kSpread = 2.0f;

  static const std::string kExplosionAnimation;

  std::vector<SpriteSheet> variants_;

  struct ActiveEmitter {
    glm::vec3 pos;
    float age = 0.0f;
    float nextEmit = 0.0f;
    float power = 4.0f;
  };

  std::vector<ActiveEmitter> emitters_;

  void emitBurst(const ActiveEmitter& em);
  const SpriteSheet& pickVariant() const;
};

}  // namespace gfx