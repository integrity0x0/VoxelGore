#pragma once

#include <span>
#include <vector>

#include "Particle.h"

namespace gfx {

class ParticleEmitter {
 public:
  ParticleEmitter() = default;

  virtual ~ParticleEmitter() = default;

  virtual void Update(float dt) = 0;

  size_t aliveCount() const { return particles_.size(); }

  std::span<const Particle> particles() const { return particles_; }

 protected:
  std::vector<Particle> particles_;

  virtual void updateParticles(float dt) {
    for (auto& p : particles_) {
      p.velocity += p.acceleration * dt;
      p.pos += p.velocity * dt;
      p.life -= dt;
      p.rotation += p.angularVelocity * dt;
    }

    particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
                                    [](const Particle& p) { return p.life <= 0.0f; }),
                     particles_.end());
  }
};

}  // namespace gfx