#include "BlockAnimationParser.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../../util/pathUtils.h"
#include "../../common/texture/SpriteSheetParser.h"
#include "../../../util/files.h"

#ifdef __ANDROID__
#include <android/asset_manager.h>
extern AAssetManager* g_AAssetManager;
#endif

namespace gfx {
namespace {

[[nodiscard]] nlohmann::json LoadJson(std::string_view path) {
  return nlohmann::json::parse(util::ReadFile(path));
}

[[nodiscard]] UvRegion ResolveFrame(const std::string& path, Atlas& atlas) {
  const AtlasRegion* region = atlas.Require(path);
  if (!region) {
    if (!atlas.Load(path)) {
      throw std::runtime_error("AnimationParser: failed to load texture: " + path);
    }
    region = atlas.Require(path);
    if (!region) {
      throw std::runtime_error("AnimationParser: texture loaded but region missing: " + path);
    }
  }
  return region->toUv();
}

[[nodiscard]] float ParseDuration(const nlohmann::json& json) {
  const float duration = json.at("duration").get<float>();
  if (duration <= 0.0f) {
    throw std::runtime_error("AnimationParser: 'duration' must be > 0");
  }
  return duration;
}

[[nodiscard]] BlockAnimation ParseFromSpriteSheet(const nlohmann::json& json, const std::string& jsonPath,
                                                  Atlas& atlas) {
  const float duration = ParseDuration(json);

  auto sheetOpt = SpriteSheetParser::parse(json.at("spriteSheet"), jsonPath, atlas);
  if (!sheetOpt) {
    throw std::runtime_error("AnimationParser: failed to parse 'spriteSheet'");
  }

  SpriteSheet sheet = std::move(*sheetOpt);
  if (sheet.frameCount() == 0) {
    throw std::runtime_error("AnimationParser: spriteSheet has no frames");
  }

  const float frameDuration = duration / static_cast<float>(sheet.frameCount());
  return BlockAnimation(std::move(sheet), frameDuration);
}

}  // namespace

BlockAnimation AnimationParser::Parse(const std::string& jsonPath, Atlas& atlas) {
  const auto json = LoadJson(jsonPath);

  if (json.contains("spriteSheet")) {
    return ParseFromSpriteSheet(json, jsonPath, atlas);
  }

  throw std::runtime_error("AnimationParser: expected 'spriteSheet' in " + jsonPath);
}

}  // namespace gfx