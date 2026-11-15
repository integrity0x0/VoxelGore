#pragma once

#include <optional>
#include <string_view>
#include <vector>

#include "SpriteSheet.h"
#include "../../../../include/nlohmann/json.hpp"

namespace gfx {

class Atlas;

class SpriteSheetParser {
 public:
  SpriteSheetParser() = delete;

  [[nodiscard]] static std::vector<SpriteSheet> ParseVariants(const nlohmann::json& j,
                                                              const std::string& jsonPath,
                                                              Atlas& atlas);

  [[nodiscard]] static std::vector<SpriteSheet> ParseVariants(std::string_view jsonPath,
                                                              Atlas& atlas);

  [[nodiscard]] static std::optional<SpriteSheet> Parse(const nlohmann::json& j,
                                                        const std::string& jsonPath, Atlas& atlas);

  [[nodiscard]] static std::optional<SpriteSheet> Parse(std::string_view jsonPath, Atlas& atlas);

 private:
  struct CommonData {
    const AtlasRegion* region = nullptr;
    glm::ivec2 frameSize{0};
    SpriteSheet::Orientation orientation = SpriteSheet::Orientation::Horizontal;
    uint32_t rows = 1;
  };

  [[nodiscard]] static std::optional<CommonData> LoadCommon(const nlohmann::json& j,
                                                            const std::string& jsonPath,
                                                            Atlas& atlas);
};

}  // namespace gfx