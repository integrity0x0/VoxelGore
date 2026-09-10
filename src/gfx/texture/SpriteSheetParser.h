#pragma once

#include <optional>
#include <string_view>
#include <vector>

#include "SpriteSheet.h"
#include "nlohmann/json.hpp"

namespace gfx {

class Atlas;

class SpriteSheetParser {
 public:
  SpriteSheetParser() = delete;

  [[nodiscard]] static std::vector<SpriteSheet> parseVariants(const nlohmann::json& j,
                                                              const std::string& jsonPath,
                                                              Atlas& atlas);

  [[nodiscard]] static std::vector<SpriteSheet> parseVariants(std::string_view jsonPath,
                                                              Atlas& atlas);

  [[nodiscard]] static std::optional<SpriteSheet> parse(const nlohmann::json& j,
                                                        const std::string& jsonPath, Atlas& atlas);

  [[nodiscard]] static std::optional<SpriteSheet> parse(std::string_view jsonPath, Atlas& atlas);

 private:
  struct CommonData {
    const AtlasRegion* region = nullptr;
    glm::ivec2 frameSize{0};
    SpriteSheet::Orientation orientation = SpriteSheet::Orientation::Horizontal;
    uint32_t rows = 1;
  };

  [[nodiscard]] static std::optional<CommonData> loadCommon(const nlohmann::json& j,
                                                            const std::string& jsonPath,
                                                            Atlas& atlas);
};

}  // namespace gfx