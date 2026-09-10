#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>
#include <queue>
#include <vector>

#include "Entity.h"

namespace gm {

class EntityManager {
 public:
  Entity CreateEntity() {
    Entity entity;

    if (!freeIndices_.empty()) {
      const EntityId id = freeIndices_.front();
      freeIndices_.pop();

      entity = Entity(id, generations_[id]);
    } else {
      const EntityId id = static_cast<EntityId>(generations_.size());

      generations_.push_back(0u);
      entity = Entity(id, 0u);
    }

    const size_t index = entities_.size();

    entities_.push_back(entity);

    ExtendIndices(entity.id);
    indices_[entity.id] = index;

    return entity;
  }

  uint32_t GetGeneration(EntityId id) const {
    return static_cast<size_t>(id) < entities_.size() ? entities_[id].generation : 0u;
  }

  void DestroyEntity(Entity entity) {
    if (!IsAlive(entity)) return;

    const size_t entityIndex = indices_[entity.id];
    const size_t lastIndex = entities_.size();

    if (entityIndex != lastIndex) {
      const Entity movedEntity = entities_.back();

      entities_[entityIndex] = movedEntity;
      indices_[movedEntity.id] = entityIndex;
    }

    entities_.pop_back();

    ++generations_[entity.id];
    freeIndices_.push(entity.id);
  }

  [[nodiscard]] bool IsAlive(Entity entity) const {
    if (entity.id == kInvalidEntityId) return false;

    const size_t index = static_cast<size_t>(static_cast<size_t>(entity.id));

    if (index >= generations_.size()) return false;

    return generations_[index] == entity.generation;
  }

  [[nodiscard]] const std::vector<Entity>& entities() const { return entities_; }

 private:
  void ExtendIndices(EntityId entityId) {
    const size_t index = static_cast<size_t>(entityId);

    if (index >= indices_.size()) {
      indices_.resize(index + 1u, kInvalidIndex);
    }
  }

 private:
  static constexpr size_t kInvalidIndex = std::numeric_limits<size_t>::max();
  std::vector<uint32_t> generations_;
  std::vector<Entity> entities_;
  std::vector<size_t> indices_;
  std::queue<EntityId> freeIndices_;
};

}  // namespace gm