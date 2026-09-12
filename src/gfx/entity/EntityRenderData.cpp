#include "EntityRenderData.h"

namespace gfx {

void EntityRenderData::Update(gm::Entity entity, const gm::RenderComponent& component) {
  Extend(entity.id);

  RenderInfo& info = renderInfos_[entity.id];

  if (info.generation == entity.generation ||
      info.generation != std::numeric_limits<uint32_t>::max()) {
    return;
  }

  info = RenderInfo{
      .generation = entity.generation,
  };

  switch (component.type) {
    case gm::RenderComponent::Type::Model:
      info.model = modelCache_->Get(modelCache_->Require(component.resource));
      break;

    case gm::RenderComponent::Type::Billboard:
      info.billboardRegion = generalAtlas_->Require(component.resource);
      break;
  }
}

void EntityRenderData::Remove(gm::Entity entity) {
  if (entity.id >= renderInfos_.size()) return;

  RenderInfo& info = renderInfos_[entity.id];

  if (info.generation != entity.generation) return;

  info.model = nullptr;
}

const EntityRenderData::RenderInfo* EntityRenderData::Get(gm::Entity entity) const {
  if (entity.id >= renderInfos_.size()) {
    return nullptr;
  }

  const RenderInfo& info = renderInfos_[entity.id];

  if (info.generation != entity.generation) {
    return nullptr;
  }

  return &info;
}

void EntityRenderData::Extend(gm::EntityId id) {
  if (id >= renderInfos_.size()) {
    const size_t oldSize = renderInfos_.size();

    renderInfos_.resize(static_cast<size_t>(id) + 1);

    for (size_t i = oldSize; i < renderInfos_.size(); ++i) {
      renderInfos_[i].generation = std::numeric_limits<uint32_t>::max();
    }
  }
}

}  // namespace gfx