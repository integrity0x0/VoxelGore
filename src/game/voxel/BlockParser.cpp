#include "BlockParser.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "../../core/PathPrefixes.h"
#include "../../util/files.h"
#include "../../util/pathUtils.h"
#include "BlockDebrisParser.h"

namespace gm {

namespace {

std::string ResolveFromAssetRoot(const std::string& logical) {
  return util::NormalizePath(core::kAssetsPrefix + logical);
}

}  // namespace

std::unordered_map<std::string, Block::RenderLayer> BlockParser::kNameByRenderLayer = {
    {"solid", Block::RenderLayer::Solid},
    {"cutout", Block::RenderLayer::Cutout},
    {"translucent", Block::RenderLayer::Translucent}};

Block BlockParser::ParseBlockJson(uint32_t id, const nlohmann::json& j,
                                  const std::string& jsonPath) {
  Block block(id);

  block.setDebrisConfig(BlockDebrisParser::parseFromBlockJson(j));
  block.setRenderGroup(j.value("renderGroup", "opaque"));
  block.setObstacle(j.value("obstacle", true));
  block.setPassingLight(j.value("passingLight", false));
  block.setHardness(j.value("hardness", 0.0f));
  block.setFriction(j.value("friction", 15.0f));
  block.setIgnoreLighting(j.value("ignoreLighting", false));
  block.setRenderLayer(kNameByRenderLayer[j.value("renderLayer", "solid")]);
  if (j.contains("emission") && j["emission"].is_array() && j["emission"].size() == 3) {
    glm::ivec3 emission{j["emission"][0].get<int>(), j["emission"][1].get<int>(),
                        j["emission"][2].get<int>()};
    block.setEmission(emission);
  }

  if (j.contains("light")) {
    const auto& lightJson = j["light"];

    if (!lightJson.is_object()) {
      throw std::runtime_error("BlockParser: block " + std::to_string(id) +
                               " 'light' must be an object");
    }

    if (!lightJson.contains("channel") || !lightJson["channel"].is_string()) {
      throw std::runtime_error("BlockParser: block " + std::to_string(id) +
                               " 'light.channel' must be a string");
    }

    const int strength = lightJson.value<int>("strength", 15);

    if (strength < 0 || strength > 15) {
      throw std::runtime_error("BlockParser: block " + std::to_string(id) +
                               " 'light.strength' must be between 0 and 15");
    }

    Block::Light light;
    light.channelId = lightJson["channel"].get<std::string>();
    light.strength = static_cast<uint8_t>(strength);

    block.setLight(light);
  }

  if (!j.contains("textures")) {
    throw std::runtime_error("BlockParser: block " + jsonPath + " has no 'textures'");
  }

  const auto& texturesJson = j["textures"];

  if (texturesJson.is_object()) {
    std::array<std::string, 6> surfaces;
    surfaces.fill("");

    auto setIfExists = [&](const std::string& key, int index) {
      if (texturesJson.contains(key) && texturesJson[key].is_string()) {
        surfaces[index] = ResolveFromAssetRoot(texturesJson[key].get<std::string>());
      }
    };

    if (texturesJson.contains("all") && texturesJson["all"].is_string()) {
      const std::string allPath = ResolveFromAssetRoot(texturesJson["all"].get<std::string>());
      for (auto& s : surfaces) s = allPath;
    }

    if (texturesJson.contains("sides") && texturesJson["sides"].is_string()) {
      const std::string sidesPath = ResolveFromAssetRoot(texturesJson["sides"].get<std::string>());
      for (int i = 0; i < 4; ++i) surfaces[i] = sidesPath;
    }

    setIfExists("north", 0);   // z-
    setIfExists("south", 1);   // z+
    setIfExists("west", 2);    // x-
    setIfExists("east", 3);    // x+
    setIfExists("bottom", 4);  // y-
    setIfExists("top", 5);     // y+

    for (int i = 0; i < 6; ++i) {
      if (surfaces[i].empty()) {
        throw std::runtime_error("BlockParser: block " + std::to_string(id) +
                                 " missing texture for face " + std::to_string(i));
      }
    }

    block.setSurfaces(surfaces);
    return block;
  }

  if (texturesJson.is_string()) {
    block.setAllSurfaces(ResolveFromAssetRoot(texturesJson.get<std::string>()));
    return block;
  }

  if (!texturesJson.is_array()) {
    throw std::runtime_error("BlockParser: block " + std::to_string(id) +
                             " 'textures' must be a string, array, or object");
  }

  if (texturesJson.empty()) {
    throw std::runtime_error("BlockParser: block " + std::to_string(id) +
                             " 'textures' array is empty");
  }

  if (texturesJson.size() == 1 && texturesJson[0].is_string()) {
    block.setAllSurfaces(ResolveFromAssetRoot(texturesJson[0].get<std::string>()));
  } else if (texturesJson.size() == 6) {
    std::array<std::string, 6u> surfaces{};
    for (size_t i = 0; i < 6; ++i) {
      if (texturesJson[i].is_string()) {
        surfaces[i] = ResolveFromAssetRoot(texturesJson[i].get<std::string>());
      }
    }
    block.setSurfaces(surfaces);
  } else {
    throw std::runtime_error("BlockParser: block " + std::to_string(id) +
                             " 'textures' must have 1 or 6 entries, got " +
                             std::to_string(texturesJson.size()));
  }

  return block;
}

Block BlockParser::Parse(uint32_t id, std::string_view path) {
  nlohmann::json j = nlohmann::json::parse(util::ReadFile(path));
  return ParseBlockJson(id, j, std::string(path));
}

}  // namespace gm