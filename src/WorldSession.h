#pragma once

#include <memory>
#include <string>

#include "game/entity/HealthSystem.h"
#include "game/World.h"
#include "game/entity/EntityDefinitionRegistry.h"
#include "game/entity/EntityFactory.h"
#include "game/lighting/Lighting.h"
#include "game/physics/CollisionResolver.h"
#include "game/physics/PhysicsSystem.h"
#include "game/voxel/BlockManager.h"

namespace gm {

class WorldSession {
 public:
  WorldSession(uint32_t width, uint32_t height, uint32_t depth, const std::string& assetsPrefix);

  void Update(float dt);

  void SetVoxel(const glm::ivec3& worldPos, uint16_t voxelId);

  World& world() { return *world_; }
  const World& world() const { return *world_; }

  BlockManager& blocks() { return *blockManager_; }
  const BlockManager& blocks() const { return *blockManager_; }

  Lighting& lighting() { return *lighting_; }
  const Lighting& lighting() const { return *lighting_; }

  CollisionResolver& collision() { return *collisionResolver_; }
  EntityFactory& entities() { return *entityFactory_; }
  gm::ComponentRegistry& components() { return world_->components(); }

 private:
  std::unique_ptr<World> world_;
  std::unique_ptr<BlockManager> blockManager_;
  std::unique_ptr<Lighting> lighting_;
  std::unique_ptr<EntityDefinitionRegistry> entityDefs_;
  std::unique_ptr<EntityFactory> entityFactory_;
  std::unique_ptr<CollisionResolver> collisionResolver_;
  std::unique_ptr<PhysicsSystem> physicsSystem_;
  std::unique_ptr<HealthSystem> healthSystem_;

};

}  // namespace gm