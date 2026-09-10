#pragma once

#include "../entity/ComponentRegistry.h"
#include "CollisionResolver.h"
#include "Hitbox.h"
#include "HitboxComponent.h"

namespace gm {
class PhysicsSystem {
 public:
  explicit PhysicsSystem(CollisionResolver& resolver) : resolver_(&resolver) {}

  void Update(ComponentRegistry& registry, float dt) {
    auto& hitboxes = registry.Storage<HitboxComponent>().denseComponents();

    for (size_t i = 0; i < hitboxes.size(); ++i) {
      resolver_->Collision(hitboxes[i], dt);
    }
  }

 private:
  CollisionResolver* resolver_;
};
}  // namespace gm