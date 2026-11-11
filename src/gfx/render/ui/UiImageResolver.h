#pragma once

#include <string>

#include "../../common/texture/MaterialManager.h"
#include "UiTypes.h"

namespace gfx {
class UiImageResolver {
 public:
  UiImageResolver(MaterialManager& materials, std::string assetRoot)
      : materials_(materials), assetRoot_(std::move(assetRoot)) {}

  [[nodiscard]] std::optional<UiTextureRegion> Resolve(std::string_view spec) const;

 private:
  MaterialManager& materials_;
  std::string assetRoot_;
};
}