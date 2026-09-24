#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <string_view>

#include "RenderComponent.h"

namespace gm {

class EntityParser {
 public:
  struct Definition {
    std::string id;

    struct Render {
      RenderComponent::Type type;
      RenderComponent::RenderLayer layer = RenderComponent::RenderLayer::Solid;
      std::string resource;
      glm::vec2 billboardSize{1.0f};
      bool ignoreLighting = false;
      bool ignoreHurtColor = false;
    };

    struct Hitbox {
      glm::vec3 size;
    };

    struct Health {
      float max;
      float start;
    };

    std::optional<Render> render;
    std::optional<Hitbox> hitbox;
    std::optional<Health> health;
    bool bleeding = false;
  };

  EntityParser() = delete;

  static std::optional<Definition> Parse(std::string_view path);
};

}  // namespace gm