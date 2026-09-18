#include "SpriteSheetParser.h"

#include <fstream>
#include <sstream>

#include "../../../util/pathUtils.h"

#if defined(__ANDROID__)
#include <android/asset_manager.h>
extern AAssetManager* g_AAssetManager;
#endif

namespace gfx {
namespace {

nlohmann::json loadJson(std::string_view path) {
#ifdef __ANDROID__
  AAsset* asset = AAssetManager_open(g_AAssetManager, path.data(), AASSET_MODE_BUFFER);
  if (!asset) {
    throw std::runtime_error("SpriteSheetParser: failed to open asset: " + std::string(path));
  }

  off_t length = AAsset_getLength(asset);
  std::vector<char> buffer(static_cast<size_t>(length));

  int readBytes = AAsset_read(asset, buffer.data(), length);
  AAsset_close(asset);

  if (readBytes != length) {
    throw std::runtime_error("SpriteSheetParser: failed to read full asset: " + std::string(path));
  }

  return nlohmann::json::parse(buffer.begin(), buffer.end());
#else
  std::ifstream f{std::string(path), std::ios::binary};
  if (!f) {
    throw std::runtime_error("SpriteSheetParser: failed to open file: " + std::string(path));
  }

  std::stringstream ss;
  ss << f.rdbuf();
  return nlohmann::json::parse(ss.str());
#endif
}

SpriteSheet::Orientation parseOrientation(const std::string& value) {
  if (value == "horizontal") return SpriteSheet::Orientation::Horizontal;
  if (value == "vertical") return SpriteSheet::Orientation::Vertical;
  throw std::runtime_error("SpriteSheetParser: unknown orientation: " + value);
}

}  // namespace

std::optional<SpriteSheetParser::CommonData> SpriteSheetParser::loadCommon(
    const nlohmann::json& j, const std::string& jsonPath, Atlas& atlas) {
  const std::string pathStr(jsonPath);
  const size_t slash = pathStr.find_last_of("/\\");
  const std::string jsonDir =
      (slash == std::string::npos) ? std::string{} : pathStr.substr(0, slash + 1);

  const std::string textureRel = j.at("texture").get<std::string>();
  const std::string texturePath = util::NormalizePath(jsonDir + textureRel);

  const AtlasRegion* region = atlas.Load(texturePath);
  if (!region) {
    return std::nullopt;
  }

  const auto& frameSizeJson = j.at("frameSize");
  glm::ivec2 frameSize(frameSizeJson.at(0).get<int>(), frameSizeJson.at(1).get<int>());

  auto orientation = parseOrientation(j.at("orientation").get<std::string>());
  uint32_t rows = j.value("rows", 1u);

  return CommonData{region, frameSize, orientation, rows};
}

std::vector<SpriteSheet> SpriteSheetParser::parseVariants(const nlohmann::json& j,
                                                          const std::string& jsonPath,
                                                          Atlas& atlas) {
  auto common = loadCommon(j, jsonPath, atlas);
  if (!common) return {};

  std::vector<SpriteSheet> result;
  result.reserve(common->rows);

  for (uint32_t row = 0; row < common->rows; ++row) {
    result.emplace_back(*common->region, common->frameSize, common->orientation, row);
  }

  return result;
}

std::vector<SpriteSheet> SpriteSheetParser::parseVariants(std::string_view jsonPath, Atlas& atlas) {
  return parseVariants(loadJson(jsonPath), std::string(jsonPath), atlas);
}

std::optional<SpriteSheet> SpriteSheetParser::parse(const nlohmann::json& j,
                                                    const std::string& jsonPath, Atlas& atlas) {
  auto common = loadCommon(j, jsonPath, atlas);
  if (!common) return std::nullopt;

  return SpriteSheet(*common->region, common->frameSize, common->orientation, 0);
}

std::optional<SpriteSheet> SpriteSheetParser::parse(std::string_view jsonPath, Atlas& atlas) {
  return parse(loadJson(jsonPath), std::string(jsonPath), atlas);
}

}  // namespace gfx