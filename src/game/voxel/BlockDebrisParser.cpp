#include "BlockDebrisParser.h"

#include <stdexcept>

namespace gm {

BlockDebrisConfig BlockDebrisParser::parseDebrisConfig(const nlohmann::json& j) {
  BlockDebrisConfig config;

  if (j.contains("count")) {
    config.count = j["count"].get<uint32_t>();
  }

  if (j.contains("size") && j["size"].is_array() && j["size"].size() >= 2) {
    config.size = {j["size"][0].get<float>(), j["size"][1].get<float>()};
  }

  if (j.contains("velocityMin") && j["velocityMin"].is_array() && j["velocityMin"].size() >= 3) {
    config.velocityMin = {j["velocityMin"][0].get<float>(), j["velocityMin"][1].get<float>(),
                          j["velocityMin"][2].get<float>()};
  }

  if (j.contains("velocityMax") && j["velocityMax"].is_array() && j["velocityMax"].size() >= 3) {
    config.velocityMax = {j["velocityMax"][0].get<float>(), j["velocityMax"][1].get<float>(),
                          j["velocityMax"][2].get<float>()};
  }

  if (j.contains("acceleration") && j["acceleration"].is_array() && j["acceleration"].size() >= 3) {
    config.acceleration = {j["acceleration"][0].get<float>(), j["acceleration"][1].get<float>(),
                           j["acceleration"][2].get<float>()};
  }

  if (j.contains("lifetime") && j["lifetime"].is_array() && j["lifetime"].size() >= 2) {
    config.lifetime = {j["lifetime"][0].get<float>(), j["lifetime"][1].get<float>()};
  }

  if (j.contains("rotationSpeed") && j["rotationSpeed"].is_array() &&
      j["rotationSpeed"].size() >= 2) {
    config.rotationSpeed = {j["rotationSpeed"][0].get<float>(), j["rotationSpeed"][1].get<float>()};
  }

  if (j.contains("bounce")) {
    config.bounce = j["bounce"].get<float>();
  }

  if (j.contains("friction")) {
    config.friction = j["friction"].get<float>();
  }

  if (j.contains("face")) {
    config.face = parseFace(j["face"].get<std::string>());
  }

  return config;
}

std::optional<BlockDebrisConfig> BlockDebrisParser::parseFromBlockJson(const nlohmann::json& j) {
  if (!j.contains("debris")) {
    return std::nullopt;
  }

  const auto& debrisJson = j["debris"];

  if (debrisJson.is_boolean() && !debrisJson.get<bool>()) {
    return std::nullopt;
  }

  if (debrisJson.is_object()) {
    return parseDebrisConfig(debrisJson);
  }

  return BlockDebrisConfig();
}

BlockDebrisConfig::Face BlockDebrisParser::parseFace(const std::string& faceStr) {
  if (faceStr == "random") {
    return BlockDebrisConfig::Face::Random;
  } else if (faceStr == "top") {
    return BlockDebrisConfig::Face::Top;
  } else if (faceStr == "bottom") {
    return BlockDebrisConfig::Face::Bottom;
  } else if (faceStr == "side") {
    return BlockDebrisConfig::Face::Side;
  } else {
    throw std::runtime_error("BlockDebrisParser: unknown face: " + faceStr);
  }
}

}  // namespace gm