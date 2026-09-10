#include "BlockParser.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "BlockDebrisParser.h"

namespace gm {

nlohmann::json BlockParser::loadJson(std::string_view path) {
#ifdef __ANDROID__
  AAsset* asset = AAssetManager_open(g_AAssetManager, path.data(), AASSET_MODE_BUFFER);
  if (!asset) {
    throw std::runtime_error("BlockParser: failed to open asset: " + std::string(path));
  }

  off_t length = AAsset_getLength(asset);
  std::vector<char> buffer(length);

  int readBytes = AAsset_read(asset, buffer.data(), length);
  AAsset_close(asset);

  if (readBytes != length) {
    throw std::runtime_error("BlockParser: failed to read full asset: " + std::string(path));
  }

  return nlohmann::json::parse(buffer.begin(), buffer.end());
#else
  std::ifstream f{std::string(path), std::ios::binary};
  if (!f) {
    throw std::runtime_error("BlockParser: failed to open file: " + std::string(path));
  }

  std::stringstream ss;
  ss << f.rdbuf();
  return nlohmann::json::parse(ss.str());
#endif
}

std::unordered_map<std::string, Block::RenderLayer> BlockParser::kNameByRenderLayer = {
    {"solid", Block::RenderLayer::Solid},
    {"cutout", Block::RenderLayer::Cutout},
    {"translucent", Block::RenderLayer::Translucent}};

Block BlockParser::parseBlockJson(uint32_t id, const nlohmann::json& j,
                                  const std::string& jsonPath) {
  Block block(id);

  block.setDebrisConfig(BlockDebrisParser::parseFromBlockJson(j));
  block.setRenderGroup(j.value("renderGroup", "opaque"));
  block.setObstacle(j.value("obstacle", false));
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

  if (!j.contains("textures")) {
    throw std::runtime_error("BlockParser: block " + std::to_string(id) + " has no 'textures'");
  }

  size_t slashPos = jsonPath.find_last_of('/');
  std::string jsonDir = (slashPos == std::string::npos) ? "" : jsonPath.substr(0, slashPos + 1);

  const auto& texturesJson = j["textures"];

  if (texturesJson.is_object()) {
    std::array<std::string, 6> surfaces;
    surfaces.fill("");

    auto setIfExists = [&](const std::string& key, int index) {
      if (texturesJson.contains(key) && texturesJson[key].is_string()) {
        surfaces[index] = util::NormalizePath(jsonDir + texturesJson[key].get<std::string>());
      }
    };

    if (texturesJson.contains("all") && texturesJson["all"].is_string()) {
      std::string allPath = util::NormalizePath(jsonDir + texturesJson["all"].get<std::string>());
      for (auto& s : surfaces) s = allPath;
    }

    if (texturesJson.contains("sides") && texturesJson["sides"].is_string()) {
      std::string sidesPath =
          util::NormalizePath(jsonDir + texturesJson["sides"].get<std::string>());
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

  // Строка или массив
  if (texturesJson.is_string()) {
    block.setAllSurfaces(util::NormalizePath(jsonDir + texturesJson.get<std::string>()));
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
    block.setAllSurfaces(util::NormalizePath(jsonDir + texturesJson[0].get<std::string>()));
  } else if (texturesJson.size() == 6) {
    std::array<std::string, 6u> surfaces{};
    for (size_t i = 0; i < 6; ++i) {
      if (texturesJson[i].is_string()) {
        surfaces[i] = util::NormalizePath(jsonDir + texturesJson[i].get<std::string>());
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
  nlohmann::json j = loadJson(path);
  return parseBlockJson(id, j, std::string(path));
}

}  // namespace gm