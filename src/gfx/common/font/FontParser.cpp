#include "FontParser.h"

#include <algorithm>
#include <cctype>
#include <nlohmann/json.hpp>
#include <unordered_map>

#include "../../../util/files.h"

namespace gfx {
std::optional<FontFamilyDefinition> FontParser::Parse(std::string_view jsonPath) {
  const auto bytes = util::ReadFileBytes(jsonPath);
  if (bytes.empty()) return std::nullopt;

  const char* begin = reinterpret_cast<const char*>(bytes.data());
  const nlohmann::json j =
      nlohmann::json::parse(begin, begin + bytes.size());
  if (j.is_discarded() || !j.is_object()) return std::nullopt;

  const auto nameIt = j.find("name");
  if (nameIt == j.end() || !nameIt->is_string()) return std::nullopt;

  const auto stylesIt = j.find("styles");
  if (stylesIt == j.end() || !stylesIt->is_array() || stylesIt->empty()) return std::nullopt;

  FontFamilyDefinition result;
  result.name = nameIt->get<std::string>();

  for (const auto& styleJson : *stylesIt) {
    auto face = ParseFace(styleJson);
    if (!face) return std::nullopt;
    result.faces.push_back(std::move(*face));
  }
  return result;
}

std::optional<FontFaceDefinition> FontParser::ParseFace(const nlohmann::json& j) {
  if (!j.is_object()) return std::nullopt;

  const auto fileIt = j.find("file");
  if (fileIt == j.end() || !fileIt->is_string()) return std::nullopt;

  FontFaceDefinition face;
  face.file = fileIt->get<std::string>();

  if (const auto it = j.find("weight"); it != j.end()) {
    auto weight = ParseWeight(*it);
    if (!weight) return std::nullopt;
    face.style.weight = *weight;
  }
  if (const auto it = j.find("italic"); it != j.end()) {
    if (!it->is_boolean()) return std::nullopt;
    face.style.italic = it->get<bool>();
  }
  return face;
}

std::optional<FontWeight> FontParser::ParseWeight(const nlohmann::json& j) {
  if (j.is_number_integer()) {
    uint32_t w = j.get<uint32_t>();
    if (w < 1 || w > 1000) return std::nullopt;
    return static_cast<FontWeight>(w);
  }
  if (!j.is_string()) return std::nullopt;

  static const std::unordered_map<std::string, FontWeight> kNames = {
      {"thin", FontWeight::Thin},           {"extralight", FontWeight::ExtraLight},
      {"light", FontWeight::Light},         {"regular", FontWeight::Regular},
      {"normal", FontWeight::Regular},      {"medium", FontWeight::Medium},
      {"semibold", FontWeight::SemiBold},   {"bold", FontWeight::Bold},
      {"extrabold", FontWeight::ExtraBold}, {"black", FontWeight::Black}};

  std::string s = j.get<std::string>();
  std::transform(s.begin(), s.end(), s.begin(),
                 [](char c) { return static_cast<char>(std::tolower(c)); });

  const auto it = kNames.find(s);
  if (it == kNames.end()) return std::nullopt;
  return it->second;
}
}  // namespace gfx