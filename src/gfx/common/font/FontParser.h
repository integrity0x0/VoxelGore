#pragma once
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "FontStyle.h"

namespace gfx {

struct FontFaceDefinition {
  FontStyle style;
  std::string file; 
};

struct FontFamilyDefinition {
  std::string name;
  std::vector<FontFaceDefinition> faces;
};

class FontParser {
 public:
  FontParser() = delete;
  [[nodiscard]] static std::optional<FontFamilyDefinition> Parse(std::string_view jsonPath);

 private:
  [[nodiscard]] static std::optional<FontFaceDefinition> ParseFace(const nlohmann::json& j);
  [[nodiscard]] static std::optional<FontWeight> ParseWeight(const nlohmann::json& j);
};
}  // namespace gfx