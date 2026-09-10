#pragma once

#include "entity/ComponentRegistry.h"
#include "entity/EntityManager.h"
#include "voxel/ChunkManager.h"

namespace gm {

class World {
 public:
  World(uint32_t width, uint32_t height, uint32_t depth) : chunkManager_(width, height, depth) {}

  Entity CreateEntity() { return entityManager_.CreateEntity(); }

  void DestroyEntity(Entity entity) {
    if (!entityManager_.IsAlive(entity)) return;
    registry_.DestroyEntity(entity.id);
    entityManager_.DestroyEntity(entity);
  }

  bool IsAlive(Entity entity) const { return entityManager_.IsAlive(entity); }
  EntityManager& entities() { return entityManager_; }
  const EntityManager& entities() const { return entityManager_; }
  ComponentRegistry& components() { return registry_; }
  ChunkManager& chunks() { return chunkManager_; }
  const ChunkManager& chunks() const { return chunkManager_; }

 private:
  EntityManager entityManager_;
  ComponentRegistry registry_;
  ChunkManager chunkManager_;
};

}  // namespace gm