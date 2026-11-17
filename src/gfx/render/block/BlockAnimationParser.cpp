#include "BlockAnimationParser.h"

#include <stdexcept>
#include <string>

#include "../../../util/files.h"
#include "../../common/texture/SpriteSheetParser.h"

namespace gfx {
namespace {

[[nodiscard]] nlohmann::json LoadJson(std::string_view path) {
  return nlohmann::json::parse(util::ReadFile(path));
}

[[nodiscard]] float ParseDuration(const nlohmann::json& json) {
  const float duration = json.at("duration").get<float>();
  if (duration <= 0.0f) {
    throw std::runtime_error("AnimationParser: 'duration' must be > 0");
  }
  return duration;
}

[[nodiscard]] BlockAnimation ParseFromSpriteSheet(const nlohmann::json& json,
                                                  const std::string& jsonPath, Atlas& atlas) {
  const float duration = ParseDuration(json);

  auto sheetOpt = SpriteSheetParser::Parse(json.at("spriteSheet"), jsonPath, atlas);
  if (!sheetOpt) {
    throw std::runtime_error("AnimationParser: failed to parse 'spriteSheet' in " + jsonPath);
  }

  SpriteSheet sheet = std::move(*sheetOpt);
  if (sheet.frameCount() == 0) {
    throw std::runtime_error("AnimationParser: spriteSheet has no frames in " + jsonPath);
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