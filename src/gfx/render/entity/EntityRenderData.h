#pragma once

#include <cstdint>
#include <limits>
#include <vector>

#include "../../../game/entity/Entity.h"
#include "../../../game/entity/RenderComponent.h"
#include "../../common/mesh/Model.h"
#include "../../common/mesh/ModelCache.h"
#include "../../common/texture/Atlas.h"

namespace gfx {

class EntityRenderData {
 public:
  static constexpr uint32_t kInvalidIndex = std::numeric_limits<uint32_t>::max();

  EntityRenderData(ModelCache& modelCache, Atlas& generalAtlas)
      : modelCache_(&modelCache), generalAtlas_(&generalAtlas) {}

  struct RenderInfo {
    uint32_t generation = 0;

    const Model* model = nullptr;
    const AtlasRegion* billboardRegion = nullptr;
  };

  void Update(gm::Entity entity, const gm::RenderComponent& component);

  void Remove(gm::Entity entity);

  [[nodiscard]] const RenderInfo* Get(gm::Entity entity) const;

 private:
  void Extend(gm::EntityId id);

 private:
  ModelCache* modelCache_;
  Atlas* generalAtlas_;

  std::vector<RenderInfo> renderInfos_;
};

}  // namespace gfx