#include "FontParser.h"

#include <nlohmann/json.hpp>

#include "../../../util/files.h"

namespace gfx {
std::optional<FontFamilyDefinition> FontParser::Parse(std::string_view filePath) {
  auto bytes = util::ReadFileBytes(filePath);

  if (bytes.empty()) return std::nullopt;

  nlohmann::json j;
  j.parse(filePath.data());
  
  FontFamilyDefinition result;

  if (j.contains("name")) {
    
  }
}
}  // namespace gfx