#include "EntityDefinitionRegistry.h"

namespace gm {

bool EntityDefinitionRegistry::Register(std::string_view path) {
  auto definition = EntityParser::Parse(path);

  if (!definition) {
    return false;
  }

  const std::string& id = definition->id;

  auto [it, inserted] = definitions_.emplace(id, std::move(*definition));

  return inserted;
}

bool EntityDefinitionRegistry::Contains(std::string_view id) const {
  return definitions_.contains(id);
}

const EntityParser::Definition* EntityDefinitionRegistry::Get(std::string_view id) const {
  const auto it = definitions_.find(id);

  if (it == definitions_.end()) {
    return nullptr;
  }

  return &it->second;
}

const EntityParser::Definition* EntityDefinitionRegistry::Require(std::string_view id) const {
  return &definitions_.at(std::string(id));
}

}  // namespace gm