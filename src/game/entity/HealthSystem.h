#pragma once

#include <vector>

#include "../World.h"
#include "ComponentRegistry.h"
#include "HealthComponent.h"

namespace gm {

class HealthSystem {
 public:
  void Update(ComponentRegistry& registry, World& world, float dt) {
    auto& storage = registry.Storage<HealthComponent>();
    const auto& entities = storage.denseEntities();
    auto& healths = storage.denseComponents();
    const auto& em = world.entities();

    deadBuffer_.clear();

    for (size_t i = 0; i < entities.size(); ++i) {
      HealthComponent& health = healths[i];

      if (health.isDead()) {
        const EntityId id = entities[i];
        deadBuffer_.emplace_back(id, em.GetGeneration(id));
      } else if (health.hurtFlash > 0.0f) {
        health.hurtFlash -= dt;
      }
    }

    for (const Entity& entity : deadBuffer_) {
      world.DestroyEntity(entity);
    }
  }

 private:
  std::vector<Entity> deadBuffer_;
};

}  // namespace gm