#include <cctype>
#include <sstream>

#include "../../../util/pathUtils.h"
#include "UiImageResolver.h"

namespace gfx {
namespace {

std::string_view Trim(std::string_view text) {
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())))
    text.remove_prefix(1);
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())))
    text.remove_suffix(1);
  return text;
}

}  // namespace

std::optional<UiImagePath> ParseImageRegion(std::string_view text) {
  text = Trim(text);
  const size_t hash = text.find('#');
  if (hash == std::string_view::npos) {
    if (text.empty()) return std::nullopt;
    return UiImagePath{std::string(text), std::nullopt};
  }

  const std::string_view path = Trim(text.substr(0, hash));
  const std::string_view rectText = Trim(text.substr(hash + 1));
  if (path.empty() || rectText.empty()) return std::nullopt;

  std::istringstream stream{std::string(rectText)};
  float x, y, width, height;
  char comma;
  if (!(stream >> x >> comma) || comma != ',') return std::nullopt;
  if (!(stream >> y >> comma) || comma != ',') return std::nullopt;
  if (!(stream >> width >> comma) || comma != ',') return std::nullopt;
  if (!(stream >> height)) return std::nullopt;
  std::string extra;
  if (stream >> extra) return std::nullopt;
  if (width <= 0.0f || height <= 0.0f) return std::nullopt;

  return UiImagePath{std::string(path), glm::vec4{x, y, width, height}};
}

std::optional<UiTextureRegion> ResolveImage(MaterialManager& materialManager,
                                            std::string_view assetRoot, const UiImagePath& path) {
  if (path.path.empty()) return std::nullopt;

  std::string key;
  if (path.path.front() == '@') {
    key = path.path.substr(1);
    if (key.empty()) return std::nullopt;
  } else {
    key = util::NormalizePath(std::string(assetRoot) + path.path);
  }

  const MaterialId id = materialManager.Require(key);
  const Material* material = materialManager.TryGet(id);
  if (!material) return std::nullopt;

  UiTextureRegion result{material->id};
  if (!path.rect) return result;

  const glm::vec2 textureSize(static_cast<float>(material->texture->GetWidth()),
                              static_cast<float>(material->texture->GetHeight()));
  if (textureSize.x <= 0.0f || textureSize.y <= 0.0f) return std::nullopt;

  const glm::vec4& rect = *path.rect;
  const float x = rect.x, y = rect.y, width = rect.z, height = rect.w;
  if (x < 0.0f || y < 0.0f || x + width > textureSize.x || y + height > textureSize.y)
    return std::nullopt;

  result.region.min = {x / textureSize.x, y / textureSize.y};
  result.region.max = {(x + width) / textureSize.x, (y + height) / textureSize.y};
  return result;
}

}  // namespace gfx