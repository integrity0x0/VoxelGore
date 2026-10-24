#pragma once

#include <vector>

#include "../World.h"
#include "ComponentRegistry.h"
#include "Entity.h"

namespace gm {

class HealthSystem {
 public:
  void Update(ComponentRegistry& registry, World& world, float dt);

 private:
  std::vector<Entity> deadBuffer_;
};

}  // namespace gm