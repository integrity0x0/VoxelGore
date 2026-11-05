#pragma once

#include <string>
#include <vector>
#include <optional>

#include "FontStyle.h"

namespace gfx {
struct FontStyleDefinition {
  FontStyle style;
  std::string path;
};

struct FontFamilyDefinition {
  std::string name;
  std::vector<FontStyle> styles;
};

class FontParser {
 public:
  FontParser() = delete;

  [[nodiscard]] static std::optional<FontFamilyDefinition> Parse(std::string_view filePath);

 private:
  [[nodiscard]] static std::optional<FontStyleDefinition> ParseStyle(const nlohmann::json& j);
};
}  // namespace gfx
