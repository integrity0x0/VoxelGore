#pragma once

#include <optional>

#include "../entity/ComponentRegistry.h"
#include "../voxel/BlockManager.h"
#include "../voxel/ChunkManager.h"
#include "HitboxComponent.h"

namespace gm {

struct AABB {
  glm::vec3 min;
  glm::vec3 max;
};

struct SweepResult {
  bool hit;
  float t;
  glm::vec3 normal;
};

struct RayCastHit {
  glm::vec3 pos;
  glm::ivec3 ipos;
  glm::vec3 normal;
  Entity entity;
};

class CollisionResolver {
 public:
  CollisionResolver(const ChunkManager& chunkManager, const BlockManager& blockManager)
      : chunkManager_(&chunkManager), blockManager_(&blockManager) {}
  void Depenetrate(Hitbox& hitbox) const;
  void Collision(Hitbox& hitbox, float dt, glm::vec3 g = {0.0f, -25.3f, 0.0f});
  std::optional<RayCastHit> Raycast(const glm::vec3& origin, const glm::vec3& dir,
                                    ComponentRegistry& registry, float maxDistance = 0.15f) const;
  bool CanPlaceBlock(const glm::ivec3& position, const gm::ComponentRegistry& components) const;

 private:
  SweepResult SweepAABB(const AABB& moving, const glm::vec3& vel, const AABB& other) const;
  AABB HitboxAABB(const glm::vec3& pos, const glm::vec3& size) const;
  AABB UnionAABB(const AABB& a, const AABB& b) const;
  bool IsSolid(const glm::ivec3& worldPos) const;
  float SampleFrictionAtFeet(const Hitbox& hitbox);
  const ChunkManager* chunkManager_;
  const BlockManager* blockManager_;
};

}  // namespace gm