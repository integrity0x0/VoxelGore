#include "AnimationParser.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../util/pathUtils.h"
#include "../texture/SpriteSheetParser.h"

#ifdef __ANDROID__
#include <android/asset_manager.h>
extern AAssetManager* g_AAssetManager;
#endif

namespace gfx::block {
namespace {

nlohmann::json loadJson(std::string_view path) {
#ifdef __ANDROID__
  AAsset* asset = AAssetManager_open(g_AAssetManager, path.data(), AASSET_MODE_BUFFER);
  if (!asset) {
    throw std::runtime_error("AnimationParser: failed to open asset: " + std::string(path));
  }

  off_t length = AAsset_getLength(asset);
  std::vector<char> buffer(static_cast<size_t>(length));

  int readBytes = AAsset_read(asset, buffer.data(), length);
  AAsset_close(asset);

  if (readBytes != length) {
    throw std::runtime_error("AnimationParser: failed to read full asset: " + std::string(path));
  }

  return nlohmann::json::parse(buffer.begin(), buffer.end());
#else
  std::ifstream f{std::string(path), std::ios::binary};
  if (!f) {
    throw std::runtime_error("AnimationParser: failed to open file: " + std::string(path));
  }

  std::stringstream ss;
  ss << f.rdbuf();
  return nlohmann::json::parse(ss.str());
#endif
}

UvRegion resolveFrame(const std::string& path, Atlas& atlas) {
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

float parseDuration(const nlohmann::json& json) {
  const float duration = json.at("duration").get<float>();
  if (duration <= 0.0f) {
    throw std::runtime_error("AnimationParser: 'duration' must be > 0");
  }
  return duration;
}

Animation parseFromSpriteSheet(const nlohmann::json& json, const std::string& jsonPath,
                               Atlas& atlas) {
  const float duration = parseDuration(json);

  auto sheetOpt = SpriteSheetParser::parse(json.at("spriteSheet"), jsonPath, atlas);
  if (!sheetOpt) {
    throw std::runtime_error("AnimationParser: failed to parse 'spriteSheet'");
  }

  SpriteSheet sheet = std::move(*sheetOpt);
  if (sheet.frameCount() == 0) {
    throw std::runtime_error("AnimationParser: spriteSheet has no frames");
  }

  const float frameDuration = duration / static_cast<float>(sheet.frameCount());
  return Animation(std::move(sheet), frameDuration);
}

}  // namespace

Animation AnimationParser::parse(const std::string& jsonPath, Atlas& atlas) {
  const auto json = loadJson(jsonPath);

  if (json.contains("spriteSheet")) {
    return parseFromSpriteSheet(json, jsonPath, atlas);
  }

  throw std::runtime_error("AnimationParser: expected 'spriteSheet' in " + jsonPath);
}

}  // namespace gfx::block