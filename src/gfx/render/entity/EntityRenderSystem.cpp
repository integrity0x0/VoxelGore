#include "EntityRenderSystem.h"

namespace gfx {
EntityRenderSystem::EntityRenderSystem(ModelCache& modelCache, Atlas& generalAtlas)
    : renderData_(modelCache, generalAtlas) {}

void EntityRenderSystem::Render(gm::ComponentRegistry& registry, ModelRenderer& modelRenderer,
                                BillboardRenderBucket& billboardRenderBucket,
                                const gm::World& world, const gm::Lighting& lighting,
                                uint32_t currentFrame) {
  static constexpr glm::vec3 kHurtColor = glm::vec3(1.0f, 0.15f, 0.15f);

  auto& renderStorage = registry.Storage<gm::RenderComponent>();
  auto& hitboxStorage = registry.Storage<gm::HitboxComponent>();
  auto& healthStorage = registry.Storage<gm::HealthComponent>();

  const auto& entities = renderStorage.denseEntities();
  auto& components = renderStorage.denseComponents();
  const auto& em = world.entities();

  for (size_t i = 0; i < entities.size(); ++i) {
    const gm::EntityId id = entities[i];
    const gm::RenderComponent& Render = components[i];

    const auto* hitbox = hitboxStorage.Get(id);
    if (!hitbox) {
      continue;
    }

    const glm::vec3 pos = hitbox->pos;

    glm::vec4 lightColor =
        Render.ignoreLighting ? glm::vec4(1.0f) : lighting.GetColor(glm::ivec3(pos));
    static glm::vec3 ambientColor(0.08f, 0.10f, 0.18f);

    glm::vec3 color = glm::vec3(lightColor) + lightColor.a * ambientColor; 

    color = glm::clamp(color, 0.0f, 1.0f);

    if (!Render.ignoreHurtColor && healthStorage.Contains(id) &&
        healthStorage.Get(id)->hurtFlash > 0.0f) {
      color *= kHurtColor;
    }

    const gm::Entity entity{id, em.GetGeneration(id)};
    renderData_.Update(entity, Render);

    const auto* renderInfo = renderData_.Get(entity);
    if (!renderInfo) {
      continue;
    }

    switch (Render.type) {
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
        instance.size = Render.billboardSize;

        billboardRenderBucket.Submit(instance, ToRenderLayer(Render.renderLayer), currentFrame);
        break;
      }
    }
  }
}
}  // namespace gfx