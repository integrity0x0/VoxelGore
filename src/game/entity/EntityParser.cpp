#include "EntityParser.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>

#ifdef __ANDROID__
#include <android/asset_manager.h>

extern AAssetManager* g_AAssetManager;
#endif

#include "../../util/pathUtils.h"

namespace gm {

namespace {

std::optional<nlohmann::json> LoadJson(std::string_view path) {
#ifdef __ANDROID__
  AAsset* asset = AAssetManager_open(g_AAssetManager, path.data(), AASSET_MODE_BUFFER);

  if (!asset) {
    return std::nullopt;
  }

  const size_t size = AAsset_getLength(asset);

  std::string data(size, '\0');

  const int64_t read = AAsset_read(asset, data.data(), size);

  AAsset_close(asset);

  if (read != static_cast<int64_t>(size)) {
    return std::nullopt;
  }

  try {
    return nlohmann::json::parse(data);
  } catch (const nlohmann::json::exception&) {
    return std::nullopt;
  }

#else

  std::ifstream file(path.data());

  if (!file) {
    return std::nullopt;
  }

  try {
    nlohmann::json json;
    file >> json;
    return json;
  } catch (const nlohmann::json::exception&) {
    return std::nullopt;
  }

#endif
}

std::optional<gm::RenderComponent::Type> ParseRenderType(std::string_view value) {
  if (value == "model") {
    return gm::RenderComponent::Type::Model;
  }

  if (value == "billboard") {
    return gm::RenderComponent::Type::Billboard;
  }

  return std::nullopt;
}

std::optional<gm::RenderComponent::RenderLayer> ParseRenderMode(std::string_view value) {
  if (value == "solid") {
    return gm::RenderComponent::RenderLayer::Solid;
  }

  if (value == "cutout") {
    return gm::RenderComponent::RenderLayer::Cutout;
  }

  if (value == "translucent") {
    return gm::RenderComponent::RenderLayer::Translucent;
  }

  return std::nullopt;
}

}  // namespace

std::optional<EntityParser::Definition> EntityParser::Parse(std::string_view path) {
  const auto json = LoadJson(path);

  if (!json) {
    return std::nullopt;
  }

  try {
    Definition definition;

    definition.id = json->at("id").get<std::string>();

    const size_t separator = path.find_last_of("/\\");

    const std::string_view jsonDir =
        separator == std::string_view::npos ? std::string_view{} : path.substr(0, separator + 1);

    if (json->contains("render")) {
      const auto& Render = json->at("render");

      const auto type = ParseRenderType(Render.at("type").get<std::string_view>());

      if (!type) {
        return std::nullopt;
      }

      const auto renderLayer = ParseRenderMode(Render.value("layer", "solid"));

      if (!renderLayer) {
        return std::nullopt;
      }

      const std::string resourcePath =
          std::string(jsonDir) + Render.at("resource").get<std::string>();

      Definition::Render renderDefinition{
          .type = *type,
          .layer = *renderLayer,
          .resource = util::NormalizePath(resourcePath),
          .billboardSize = glm::vec2{1.0f},
          .ignoreLighting = Render.value("ignoreLighting", false),
          .ignoreHurtColor = Render.value("ignoreHurtColor", false),
      };

      if (Render.contains("size")) {
        const auto& size = Render.at("size");

        renderDefinition.billboardSize = glm::vec2{
            size.at(0).get<float>(),
            size.at(1).get<float>(),
        };
      }

      definition.render = std::move(renderDefinition);
    }

    if (json->contains("hitbox")) {
      const auto& hitbox = json->at("hitbox");
      const auto& size = hitbox.at("size");

      definition.hitbox = Definition::Hitbox{
          .size =
              glm::vec3{
                  size.at(0).get<float>(),
                  size.at(1).get<float>(),
                  size.at(2).get<float>(),
              },
      };
    }

    if (json->contains("health")) {
      const auto& health = json->at("health");

      const float max = health.at("max").get<float>();

      definition.health = Definition::Health{
          .max = max,
          .start = health.value("start", max),
      };
    }

    return definition;

  } catch (const nlohmann::json::exception&) {
    return std::nullopt;
  }
}

}  // namespace gm