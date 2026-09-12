#include "EntityFactory.h"

#include "../physics/HitboxComponent.h"
#include "HealthComponent.h"
#include "RenderComponent.h"

namespace gm {

EntityFactory::EntityFactory(EntityManager& entityManager, ComponentRegistry& components,
                             EntityDefinitionRegistry& definitions)
    : entityManager_(&entityManager), components_(&components), definitions_(&definitions) {}

Entity EntityFactory::Create(std::string_view definitionId, const glm::vec3& position) {
  const auto* definition = definitions_->Get(definitionId);

  if (!definition) {
    return {};
  }

  Entity entity = entityManager_->CreateEntity();

  if (definition->render) {
    const auto& Render = *definition->render;

    components_->Storage<RenderComponent>().Add(entity.id,
                                                RenderComponent{
                                                    .type = Render.type,
                                                    .renderLayer = Render.layer,
                                                    .resource = Render.resource,
                                                    .billboardSize = Render.billboardSize,
                                                    .ignoreLighting = Render.ignoreLighting,
                                                    .ignoreHurtColor = Render.ignoreHurtColor,
                                                });
  }

  if (definition->hitbox) {
    components_->Storage<HitboxComponent>().Add(entity.id, HitboxComponent{
                                                               .pos = position,
                                                               .size = definition->hitbox->size,
                                                               .vel = glm::vec3(0.0f),
                                                           });
  }

  if (definition->health) {
    components_->Storage<HealthComponent>().Add(entity.id, HealthComponent{
                                                               .current = definition->health->start,
                                                               .max = definition->health->max,
                                                           });
  }

  return entity;
}

}  // namespace gm