#pragma once

#include <optional>
#include <string>

#include "UiTypes.h"

namespace gfx {

enum class UiState { Normal, Hovered, Unhovered, Held, Entry, Exit, Pressed, Released, Count };

struct UiStateOverrides {
  std::optional<UiPadding> padding;
  std::optional<glm::vec4> bgColor;
  std::optional<UiTextureRegion> bgImage;
  std::optional<UiTextureRegion> image;
  std::optional<glm::vec4> color;

  std::wstring text;
  float scale = 1.0f;
};

}  // namespace gfx