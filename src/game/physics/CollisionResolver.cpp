#include "CollisionResolver.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace gm {

AABB CollisionResolver::HitboxAABB(const glm::vec3& pos, const glm::vec3& size) const {
  glm::vec3 half = size * 0.5f;
  return {pos - half, pos + half};
}

AABB CollisionResolver::UnionAABB(const AABB& a, const AABB& b) const {
  return {glm::min(a.min, b.min), glm::max(a.max, b.max)};
}

bool CollisionResolver::IsSolid(const glm::ivec3& worldPos) const {
  return chunkManager_->hasVoxel(worldPos) && chunkManager_->getVoxel(worldPos)->id != 0;
}

static bool AabbOverlap(const AABB& a, const AABB& b) {
  return a.min.x < b.max.x && a.max.x > b.min.x && a.min.y < b.max.y && a.max.y > b.min.y &&
         a.min.z < b.max.z && a.max.z > b.min.z;
}

SweepResult CollisionResolver::SweepAABB(const AABB& moving, const glm::vec3& vel,
                                         const AABB& other) const {
  glm::vec3 invEntry, invExit;

  for (int i = 0; i < 3; ++i) {
    if (vel[i] > 0.0f) {
      invEntry[i] = other.min[i] - moving.max[i];
      invExit[i] = other.max[i] - moving.min[i];
    } else {
      invEntry[i] = other.max[i] - moving.min[i];
      invExit[i] = other.min[i] - moving.max[i];
    }
  }

  glm::vec3 entry, exit;
  for (int i = 0; i < 3; ++i) {
    if (std::abs(vel[i]) < 1e-8f) {
      if (moving.max[i] <= other.min[i] || moving.min[i] >= other.max[i]) return {false, 1.f, {}};

      entry[i] = -std::numeric_limits<float>::infinity();
      exit[i] = std::numeric_limits<float>::infinity();

    } else {
      entry[i] = invEntry[i] / vel[i];
      exit[i] = invExit[i] / vel[i];
    }
  }

  float entryTime = std::max({entry.x, entry.y, entry.z});
  float exitTime = std::min({exit.x, exit.y, exit.z});

  if (entryTime > exitTime || entryTime < 0.0f || entryTime > 1.0f) {
    return {false, 1.0f, glm::vec3(0.0f)};
  }

  constexpr float axisEps = 1e-6f;

  glm::vec3 normal(0.0f);

  if (std::abs(entry.y - entryTime) < axisEps) {
    normal.y = vel.y > 0.0f ? -1.0f : 1.0f;
  } else if (std::abs(entry.x - entryTime) < axisEps) {
    normal.x = vel.x > 0.0f ? -1.0f : 1.0f;
  } else {
    normal.z = vel.z > 0.0f ? -1.0f : 1.0f;
  }

  return {true, entryTime, normal};
}

void CollisionResolver::Depenetrate(Hitbox& hitbox) const {
  constexpr float pushEps = 1e-4f;
  constexpr int MAX_DEPEN_ITER = 4;

  for (int pass = 0; pass < MAX_DEPEN_ITER; ++pass) {
    AABB current = HitboxAABB(hitbox.pos, hitbox.size);
    glm::ivec3 minVoxel = glm::ivec3(glm::floor(current.min));
    glm::ivec3 maxVoxel = glm::ivec3(glm::floor(current.max));

    bool anyPush = false;

    for (int x = minVoxel.x; x <= maxVoxel.x && !anyPush; ++x) {
      for (int y = minVoxel.y; y <= maxVoxel.y && !anyPush; ++y) {
        for (int z = minVoxel.z; z <= maxVoxel.z && !anyPush; ++z) {
          glm::ivec3 vp{x, y, z};
          if (!IsSolid(vp)) continue;

          AABB voxelAABB{glm::vec3(vp), glm::vec3(vp) + glm::vec3(1.0f)};
          if (!AabbOverlap(current, voxelAABB)) continue;

          float overlapX =
              std::min(current.max.x, voxelAABB.max.x) - std::max(current.min.x, voxelAABB.min.x);
          float overlapY =
              std::min(current.max.y, voxelAABB.max.y) - std::max(current.min.y, voxelAABB.min.y);
          float overlapZ =
              std::min(current.max.z, voxelAABB.max.z) - std::max(current.min.z, voxelAABB.min.z);

          if (overlapX <= overlapY && overlapX <= overlapZ) {
            float dir = (current.min.x + current.max.x) < (voxelAABB.min.x + voxelAABB.max.x)
                            ? -1.0f
                            : 1.0f;
            hitbox.pos.x += dir * (overlapX + pushEps);
            hitbox.vel.x = 0.0f;
          } else if (overlapY <= overlapX && overlapY <= overlapZ) {
            float dir = (current.min.y + current.max.y) < (voxelAABB.min.y + voxelAABB.max.y)
                            ? -1.0f
                            : 1.0f;
            hitbox.pos.y += dir * (overlapY + pushEps);
            hitbox.vel.y = 0.0f;
            if (dir > 0.0f) hitbox.grounded = true;
          } else {
            float dir = (current.min.z + current.max.z) < (voxelAABB.min.z + voxelAABB.max.z)
                            ? -1.0f
                            : 1.0f;
            hitbox.pos.z += dir * (overlapZ + pushEps);
            hitbox.vel.z = 0.0f;
          }

          anyPush = true;
        }
      }
    }

    if (!anyPush) break;
  }
}

void CollisionResolver::Collision(Hitbox& hitbox, float dt, glm::vec3 g) {
  hitbox.vel += g * dt;
  hitbox.grounded = false;

  constexpr float epsilon = 1e-5f;
  constexpr int MAX_ITERATIONS = 3;

  float remainingDt = dt;

  for (int iter = 0; iter < MAX_ITERATIONS && remainingDt > 0.0f; ++iter) {
    glm::vec3 displacement = hitbox.vel * remainingDt;

    if (glm::dot(displacement, displacement) < 1e-12f) break;

    AABB current = HitboxAABB(hitbox.pos, hitbox.size);
    AABB target = HitboxAABB(hitbox.pos + displacement, hitbox.size);
    AABB motion = UnionAABB(current, target);

    glm::ivec3 minVoxel = glm::ivec3(glm::floor(motion.min));
    glm::ivec3 maxVoxel = glm::ivec3(glm::floor(motion.max));

    float bestT = 1.0f;
    glm::vec3 bestNormal(0.0f);
    bool anyHit = false;

    for (int x = minVoxel.x; x <= maxVoxel.x; ++x) {
      for (int y = minVoxel.y; y <= maxVoxel.y; ++y) {
        for (int z = minVoxel.z; z <= maxVoxel.z; ++z) {
          glm::ivec3 vp{x, y, z};
          if (!IsSolid(vp)) continue;

          AABB voxelAABB{glm::vec3(vp), glm::vec3(vp) + glm::vec3(1.0f)};

          if (AabbOverlap(current, voxelAABB)) Depenetrate(hitbox);

          SweepResult res = SweepAABB(current, displacement, voxelAABB);

          if (res.hit) {
            if (res.t < bestT) {
              bestT = res.t;
              bestNormal = res.normal;
              anyHit = true;
            }
          }
        }
      }
    }

    if (anyHit) {
      float safeT = std::max(bestT - epsilon, 0.0f);
      float dtThisStep = remainingDt * bestT;

      hitbox.pos += displacement * safeT;

      if (bestNormal.x != 0.0f) hitbox.vel.x = 0.0f;
      if (bestNormal.y != 0.0f) hitbox.vel.y = 0.0f;
      if (bestNormal.z != 0.0f) hitbox.vel.z = 0.0f;

      if (bestNormal.y > 0.0f) hitbox.grounded = true;

      hitbox.pos += bestNormal * epsilon;

      float friction = SampleFrictionAtFeet(hitbox);
      float damping = std::exp(-friction * dtThisStep);
      hitbox.vel.x *= damping;
      hitbox.vel.z *= damping;

      remainingDt *= (1.0f - bestT);
    } else {
      hitbox.pos += displacement;

      float friction = SampleFrictionAtFeet(hitbox);
      float damping = std::exp(-friction * remainingDt);
      hitbox.vel.x *= damping;
      hitbox.vel.z *= damping;

      remainingDt = 0.0f;
    }
  }
}

float CollisionResolver::SampleFrictionAtFeet(const Hitbox& hitbox) {
  constexpr float epsilon = 0.01f;

  const float halfX = hitbox.size.x * 0.5f - epsilon;
  const float halfZ = hitbox.size.z * 0.5f - epsilon;

  const glm::vec3 feet[4] = {
      hitbox.pos + glm::vec3(-halfX, -hitbox.size.y * 0.5f - epsilon, -halfZ),
      hitbox.pos + glm::vec3(halfX, -hitbox.size.y * 0.5f - epsilon, -halfZ),
      hitbox.pos + glm::vec3(-halfX, -hitbox.size.y * 0.5f - epsilon, halfZ),
      hitbox.pos + glm::vec3(halfX, -hitbox.size.y * 0.5f - epsilon, halfZ),
  };

  float friction = 0.0f;
  bool foundSurface = false;

  for (const glm::vec3& point : feet) {
    const glm::ivec3 voxelPos = glm::ivec3(point);

    if (!chunkManager_->hasVoxel(voxelPos)) {
      continue;
    }

    const auto voxel = chunkManager_->getVoxel(voxelPos);
    if (!voxel || !voxel->id) {
      continue;
    }

    const Block* block = blockManager_->block(voxel->id);
    if (!block) {
      continue;
    }

    foundSurface = true;
    friction = std::max(friction, block->friction());
  }

  if (!foundSurface && blockManager_->block(0)) {
    return blockManager_->block(0)->friction();
  }

  return friction;
}

std::optional<RayCastHit> CollisionResolver::Raycast(const glm::vec3& origin, const glm::vec3& dir,
                                                     ComponentRegistry& registry,
                                                     float maxDistance) const {
  glm::vec3 disp = glm::normalize(dir) * maxDistance;
  AABB point{origin, origin};

  float bestT = 1.0f;
  glm::vec3 bestNormal(0.0f);
  glm::ivec3 bestVoxel(0);
  Entity bestEntity = Entity();
  bool anyHit = false;

  glm::vec3 segMin = glm::min(origin, origin + disp);
  glm::vec3 segMax = glm::max(origin, origin + disp);
  glm::ivec3 minVoxel = glm::ivec3(glm::floor(segMin));
  glm::ivec3 maxVoxel = glm::ivec3(glm::floor(segMax));

  for (int x = minVoxel.x; x <= maxVoxel.x; ++x) {
    for (int y = minVoxel.y; y <= maxVoxel.y; ++y) {
      for (int z = minVoxel.z; z <= maxVoxel.z; ++z) {
        glm::ivec3 vp{x, y, z};
        if (!IsSolid(vp)) continue;

        AABB voxelAABB{glm::vec3(vp), glm::vec3(vp) + 1.0f};
        SweepResult res = SweepAABB(point, disp, voxelAABB);

        if (res.hit && res.t < bestT) {
          bestT = res.t;
          bestNormal = res.normal;
          bestVoxel = vp;
          bestEntity = Entity();
          anyHit = true;
        }
      }
    }
  }

  auto& storage = registry.Storage<HitboxComponent>();
  auto& hitboxes = storage.denseComponents();
  const auto& entities = storage.denseEntities();

  for (size_t i = 0; i < hitboxes.size(); ++i) {
    const HitboxComponent& hb = hitboxes[i];
    AABB entityAABB = HitboxAABB(hb.pos, hb.size);

    SweepResult res = SweepAABB(point, disp, entityAABB);

    if (res.hit && res.t < bestT) {
      bestT = res.t;
      bestNormal = res.normal;
      bestVoxel = glm::ivec3(0);
      bestEntity = Entity(entities[i], 0);
      anyHit = true;
    }
  }

  if (!anyHit) return std::nullopt;

  return RayCastHit{origin + disp * bestT, bestVoxel, bestNormal, bestEntity};
}

bool CollisionResolver::CanPlaceBlock(const glm::ivec3& position,
                                      const gm::ComponentRegistry& components) const {
  const AABB blockBox{
      glm::vec3(position),
      glm::vec3(position) + glm::vec3(1.0f),
  };

  auto& storage = components.Storage<gm::HitboxComponent>();

  for (const gm::EntityId entity : storage.denseEntities()) {
    const auto& hitbox = storage.Get(entity);

    if (!hitbox) {
      continue;
    }

    if (AabbOverlap(HitboxAABB(hitbox->pos, hitbox->size), blockBox)) {
      return false;
    }
  }

  return true;
}

}  // namespace gm
