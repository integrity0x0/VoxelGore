#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <string_view>

#include "../../common/texture/MaterialManager.h"
#include "UiTypes.h"

namespace gfx {

struct UiImagePath {
  std::string path;
  std::optional<glm::vec4> rect;  // x, y, w, h in pixels
};

[[nodiscard]] extern std::optional<UiImagePath> ParseImageRegion(std::string_view text);

[[nodiscard]] extern std::optional<UiTextureRegion> ResolveImage(MaterialManager& materialManager,
                                                                 std::string_view assetRoot,
                                                                 const UiImagePath& path);

}  // namespace gfx