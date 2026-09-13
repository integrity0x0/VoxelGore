#include "WorldSession.h"

#include <chrono>

#include "core/PathPrefixes.h"

namespace gm {

WorldSession::WorldSession(uint32_t width, uint32_t height, uint32_t depth,
                           const std::string& assetsPrefix) {
  world_ = std::make_unique<World>(width, height, depth);

  entityDefs_ = std::make_unique<EntityDefinitionRegistry>();
  entityFactory_ =
      std::make_unique<EntityFactory>(world_->entities(), world_->components(), *entityDefs_);

  blockManager_ = std::make_unique<BlockManager>();
  blockManager_->Load(assetsPrefix + "blocks/air.json");
  blockManager_->Load(assetsPrefix + "blocks/stone.json");
  blockManager_->Load(assetsPrefix + "blocks/red_lamp.json");
  blockManager_->Load(assetsPrefix + "blocks/glass_lime.json");
  blockManager_->Load(assetsPrefix + "blocks/glass_purple.json");
  blockManager_->Load(assetsPrefix + "blocks/sea_lantern.json");
  blockManager_->Load(assetsPrefix + "blocks/glowstone.json");
  blockManager_->Load(assetsPrefix + "blocks/ice.json");
  blockManager_->Load(assetsPrefix + "blocks/tnt.json");

  lighting_ = std::make_unique<lighting::Lighting>(world_->chunks(), *blockManager_);
  lighting_->LightUp();

  collisionResolver_ = std::make_unique<CollisionResolver>(world_->chunks(), *blockManager_);
  physicsSystem_ = std::make_unique<PhysicsSystem>(*collisionResolver_);
  healthSystem_ = std::make_unique<HealthSystem>();

  entityDefs_->Register(assetsPrefix + "entities/barrel.json");
  entityDefs_->Register(assetsPrefix + "entities/integrity.json");
}

void WorldSession::Update(float dt) {
  healthSystem_->Update(world_->components(), *world_, dt);
  physicsSystem_->Update(world_->components(), dt);
}

void WorldSession::SetVoxel(const glm::ivec3& worldPos, uint16_t voxelId) {
  const auto t0 = std::chrono::high_resolution_clock::now();

  Voxel v{};
  v.id = voxelId;
  world_->chunks().setVoxel(worldPos, v);

  const auto t1 = std::chrono::high_resolution_clock::now();
  lighting_->OnVoxelSetted(worldPos, {voxelId});
  const auto t2 = std::chrono::high_resolution_clock::now();

}

}  // namespace gm