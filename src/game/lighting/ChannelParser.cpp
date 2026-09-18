#include "ChannelParser.h"

#include <nlohmann/json.hpp>

#include "../../util/files.h"

namespace gm {
std::optional<ChannelDefinition> ChannelParser::Parse(std::string_view path) {
  auto bytes = util::ReadFileBytes(path);

  nlohmann::json json;
  try {
    json = nlohmann::json::parse(reinterpret_cast<const char*>(bytes.data()),
                                 reinterpret_cast<const char*>(bytes.data() + bytes.size()));
  } catch (const nlohmann::json::parse_error&) {
    return std::nullopt;
  }

  if (!json.contains("id") || !json["id"].is_string()) {
    return std::nullopt;
  }
  if (!json.contains("color") || !json["color"].is_array() || json["color"].size() != 3) {
    return std::nullopt;
  }

  glm::vec3 color;
  for (int i = 0; i < 3; ++i) {
    if (!json["color"][i].is_number()) return std::nullopt;
    color[i] = json["color"][i].get<float>();
  }

  return ChannelDefinition(json["id"].get<std::string>(), color);
}
}  // namespace gm