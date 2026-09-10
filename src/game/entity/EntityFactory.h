#pragma once

#include <glm/vec3.hpp>
#include <string_view>

#include "ComponentRegistry.h"
#include "EntityDefinitionRegistry.h"
#include "EntityManager.h"

namespace gm {

class EntityFactory {
 public:
  EntityFactory(EntityManager& entityManager, ComponentRegistry& components,
                EntityDefinitionRegistry& definitions);

  Entity Create(std::string_view definitionId, const glm::vec3& position);

 private:
  EntityManager* entityManager_;
  ComponentRegistry* components_;
  EntityDefinitionRegistry* definitions_;
};

}  // namespace gm