#include "SpriteSheetParser.h"

#include <stdexcept>

#include "../../../core/PathPrefixes.h"
#include "../../../util/files.h"
#include "../../../util/pathUtils.h"

namespace gfx {
namespace {

SpriteSheet::Orientation ParseOrientation(const std::string& value) {
  if (value == "horizontal") return SpriteSheet::Orientation::Horizontal;
  if (value == "vertical") return SpriteSheet::Orientation::Vertical;
  throw std::runtime_error("SpriteSheetParser: unknown orientation: " + value);
}

std::string ResolveFromAssetRoot(const std::string& logical) {
  return util::NormalizePath(core::kAssetsPrefix + logical);
}

}  // namespace

std::optional<SpriteSheetParser::CommonData> SpriteSheetParser::LoadCommon(
    const nlohmann::json& j, const std::string& /*jsonPath*/, Atlas& atlas) {
  const std::string texturePath = ResolveFromAssetRoot(j.at("texture").get<std::string>());

  const AtlasRegion* region = atlas.Load(texturePath);
  if (!region) {
    return std::nullopt;
  }

  const auto& frameSizeJson = j.at("frameSize");
  glm::ivec2 frameSize(frameSizeJson.at(0).get<int>(), frameSizeJson.at(1).get<int>());

  auto orientation = ParseOrientation(j.at("orientation").get<std::string>());
  uint32_t rows = j.value("rows", 1u);

  return CommonData{region, frameSize, orientation, rows};
}

std::vector<SpriteSheet> SpriteSheetParser::ParseVariants(const nlohmann::json& j,
                                                          const std::string& jsonPath,
                                                          Atlas& atlas) {
  auto common = LoadCommon(j, jsonPath, atlas);
  if (!common) return {};

  std::vector<SpriteSheet> result;
  result.reserve(common->rows);

  for (uint32_t row = 0; row < common->rows; ++row) {
    result.emplace_back(*common->region, common->frameSize, common->orientation, row);
  }

  return result;
}

std::vector<SpriteSheet> SpriteSheetParser::ParseVariants(std::string_view jsonPath, Atlas& atlas) {
  return ParseVariants(nlohmann::json::parse(util::ReadFile(jsonPath)), std::string(jsonPath),
                       atlas);
}

std::optional<SpriteSheet> SpriteSheetParser::Parse(const nlohmann::json& j,
                                                    const std::string& jsonPath, Atlas& atlas) {
  auto common = LoadCommon(j, jsonPath, atlas);
  if (!common) return std::nullopt;

  return SpriteSheet(*common->region, common->frameSize, common->orientation, 0);
}

std::optional<SpriteSheet> SpriteSheetParser::Parse(std::string_view jsonPath, Atlas& atlas) {
  return Parse(nlohmann::json::parse(util::ReadFile(jsonPath)), std::string(jsonPath), atlas);
}

}  // namespace gfx