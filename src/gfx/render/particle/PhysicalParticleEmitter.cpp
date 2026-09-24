#include "PhysicalParticleEmitter.h"

namespace gfx {

PhysicalParticleEmitter::PhysicalParticleEmitter(const gm::ChunkManager& chunkManager,
                                                 const gm::BlockManager& blockManager) 
    : chunkManager_(&chunkManager), blockManager_(&blockManager) {}

bool PhysicalParticleEmitter::IsObstacle(const glm::ivec3& pos) const {
  std::optional<gm::Voxel> vox = chunkManager_->getVoxel(pos);
  return vox && blockManager_->block(vox->id) && blockManager_->block(vox->id)->isObstacle();
}

void PhysicalParticleEmitter::ResolveCollision(Particle& p, float dt) {
  static constexpr float kSettleVelocity = 0.05f;
  if (p.settled_) return;

  glm::vec3 delta = p.velocity * dt;
  bool wasFalling = p.velocity.y < 0.0f;
  bool hitGround = false;

  for (int axis = 0; axis < 3; ++axis) {
    glm::vec3 next = p.pos;
    next[axis] += delta[axis];

    if (IsObstacle(glm::ivec3(glm::floor(next)))) {
      p.velocity[axis] *= -p.bounceFactor;

      if (axis == 1 && wasFalling) hitGround = true;
      continue;
    }
    p.pos[axis] = next[axis];
  }

  if (hitGround && std::abs(p.velocity.y) < kSettleVelocity) {
    p.velocity = glm::vec3(0.0f);
    p.settled_ = true;
  }
}

void PhysicalParticleEmitter::UpdateParticles(float dt) {
  for (auto& p : particles_) {
    ResolveCollision(p, dt);
    p.life -= dt;
    p.rotation += p.angularVelocity * dt;
  }

  particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
                                  [](const Particle& p) { return p.life <= 0.0f; }),
                   particles_.end());
}
}  // namespace gfx