#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "../../game/World.h"
#include "../../game/entity/ComponentRegistry.h"
#include "../../game/entity/HealthSystem.h"
#include "../../game/physics/HitboxComponent.h"
#include "../../game/voxel/ChunkManager.h"
#include "../render/ModelRenderer.h"
#include "../render/billboard/BillboardRenderer.h"
#include "EntityRenderData.h"

namespace gfx {

class EntityRenderSystem {
 public:
  EntityRenderSystem(ModelCache& modelCache, Atlas& generalAtlas);

  void Render(gm::ComponentRegistry& registry, ModelRenderer& modelRenderer,
              BillboardRenderBucket& billboardRenderBucket, const gm::World& world,
              uint32_t currentFrame);

 private:
  EntityRenderData renderData_;
};

}  // namespace gfx