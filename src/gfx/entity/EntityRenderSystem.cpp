#include "EntityRenderSystem.h"

namespace gfx {
EntityRenderSystem::EntityRenderSystem(ModelCache& modelCache, Atlas& generalAtlas)
    : renderData_(modelCache, generalAtlas) {}

void EntityRenderSystem::Render(gm::ComponentRegistry& registry, ModelRenderer& modelRenderer,
                                BillboardRenderBucket& billboardRenderBucket,
                                const gm::World& world, uint32_t currentFrame) {
  static constexpr glm::vec3 kHurtColor = glm::vec3(1.0f, 0.15f, 0.15f);

  auto& renderStorage = registry.Storage<gm::RenderComponent>();
  auto& hitboxStorage = registry.Storage<gm::HitboxComponent>();
  auto& healthStorage = registry.Storage<gm::HealthComponent>();

  const auto& entities = renderStorage.denseEntities();
  auto& components = renderStorage.denseComponents();
  const auto& em = world.entities();

  for (size_t i = 0; i < entities.size(); ++i) {
    const gm::EntityId id = entities[i];
    const gm::RenderComponent& render = components[i];

    const auto* hitbox = hitboxStorage.Get(id);
    if (!hitbox) {
      continue;
    }

    const glm::vec3 pos = hitbox->pos;

    glm::vec3 color =
        render.ignoreLighting ? glm::vec3(1.0f) : world.chunks().getLightColor(glm::ivec3(pos));

    if (!render.ignoreHurtColor && healthStorage.Contains(id) &&
        healthStorage.Get(id)->hurtFlash > 0.0f) {
      color *= kHurtColor;
    }

    const gm::Entity entity{id, em.GetGeneration(id)};
    renderData_.Update(entity, render);

    const auto* renderInfo = renderData_.Get(entity);
    if (!renderInfo) {
      continue;
    }

    switch (render.type) {
      case gm::RenderComponent::Type::Model: {
        if (!renderInfo->model) {
          continue;
        }

        const glm::mat4x3 transform = glm::mat4x3(glm::translate(glm::mat4(1.0f), pos));

        modelRenderer.Submit(*renderInfo->model, transform, glm::vec4(color, 1.0f));
        break;
      }

      case gm::RenderComponent::Type::Billboard: {
        if (!renderInfo->billboardRegion) {
          continue;
        }

        const UvRegion region = renderInfo->billboardRegion->toUv();

        BillboardInstance instance;
        instance.pos = pos;
        instance.uvMinMax = glm::vec4(region.min, region.max);
        instance.color = glm::vec4(color, 1.0f);
        instance.size = render.billboardSize;

        billboardRenderBucket.Submit(instance, ToRenderLayer(render.renderLayer), currentFrame);
        break;
      }
    }
  }
}
}  // namespace gfx