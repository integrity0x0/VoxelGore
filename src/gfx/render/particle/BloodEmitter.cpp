#include "BloodEmitter.h"

namespace gfx {
BloodEmitter::BloodEmitter(const gm::BlockManager& blockManager,
                           const gm::ChunkManager& chunkManager) 
    : blockManager_(&blockManager),
      chunkManager_(&chunkManager) {}

bool BloodEmitter::IsObstacle(const glm::ivec3& pos) {
  return chunkManager.hasVoxel(pos) && chunkManager.getVoxel(pos)->id != 0;
}

void BloodEmitter::ResolveParticleCollision(Particle& p, float dt) {
  static constexpr float kSettleVelocity = 0.05f;
  if (p.settled_) return;

  glm::vec3 delta = p.velocity * dt;
  bool wasFalling = p.velocity.y < 0.0f;
  bool hitGround = false;

  for (int axis = 0; axis < 3; ++axis) {
    glm::vec3 next = p.pos;
    next[axis] += delta[axis];

    if (IsObstacle(glm::ivec3(glm::floor(next)), chunkManager)) {
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


}  // namespace gfx