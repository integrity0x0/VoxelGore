#include "HealthSystem.h"

#include <glm/glm.hpp>

#include "../physics/HitboxComponent.h"
#include "HealthComponent.h"
#include "PlayerComponent.h"

namespace gm {

void HealthSystem::Update(ComponentRegistry& registry, World& world, float dt) {
  auto& storage = registry.Storage<HealthComponent>();
  const auto& entities = storage.GetDenseEntities();
  auto& healths = storage.GetDenseComponents();
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

  auto& playerStorage = registry.Storage<PlayerComponent>();
  auto& hitboxStorage = registry.Storage<HitboxComponent>();
  auto& healthStorage = registry.Storage<HealthComponent>();

  for (const Entity& entity : deadBuffer_) {
    if (playerStorage.Contains(entity.id)) {
      auto* player = playerStorage.Get(entity.id);
      auto* hitbox = hitboxStorage.Get(entity.id);
      auto* health = healthStorage.Get(entity.id);

      if (player && hitbox && health) {
        hitbox->pos = player->spawn;
        hitbox->vel = glm::vec3(0.0f);

        health->Revive();
        health->hurtFlash = 0.0f;
      }
    } else {
      world.DestroyEntity(entity);
    }
  }
}

}  // namespace gm