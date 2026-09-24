#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "../../../game/World.h"
#include "../../../game/entity/ComponentRegistry.h"
#include "../../../game/entity/HealthSystem.h"
#include "../../../game/physics/HitboxComponent.h"
#include "../../../game/entity/BleedComponent.h"
#include "../../../game/voxel/ChunkManager.h"
#include "../../../game/lighting/Lighting.h"
#include "../model/ModelRenderer.h"
#include "../billboard/BillboardRenderer.h"
#include "../../../game/Enviroment.h"
#include "EntityRenderData.h"
#include "../particle/ParticleEngine.h"

namespace gfx {

class EntityRenderSystem {
 public:
  EntityRenderSystem(ModelCache& modelCache, Atlas& generalAtlas);

  void Render(gm::ComponentRegistry& registry, ModelRenderer& modelRenderer,
              ParticleEngine& particleEngine,
              BillboardRenderBucket& billboardRenderBucket, const gm::World& world,
              const gm::Lighting& lighting, const gm::Enviroment& enviroment, 
              uint32_t currentFrame);

 private:
  EntityRenderData renderData_;
};

}  // namespace gfx