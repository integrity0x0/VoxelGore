#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "../../util/hashers.h"
#include "EntityParser.h"

namespace gm {

class EntityDefinitionRegistry {
 public:
  bool Register(std::string_view path);

  bool Contains(std::string_view id) const;

  const EntityParser::Definition* Get(std::string_view id) const;

  const EntityParser::Definition* Require(std::string_view id) const;

 private:
  std::unordered_map<std::string, EntityParser::Definition, util::StringHash, std::equal_to<>>
      definitions_;
};

}  // namespace gm