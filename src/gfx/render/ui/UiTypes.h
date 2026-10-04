#pragma once

#include <cstdint>
#include <utility>
#include <glm/glm.hpp>

#include "../../UvRegion.h"
#include "../../common/texture/MaterialManager.h"

namespace gfx {

enum class UiAlignX { Left, Center, Right, Count };
enum class UiAlignY { Top, Center, Bottom, Count };

struct UiRect {
  glm::vec2 pos, size;
};

struct UiPoint {
  UiAlignX x = UiAlignX::Left;
  UiAlignY y = UiAlignY::Top;
};

enum class UiUnit : uint8_t { Px, Percent };

struct UiLength {
  float value = 0.0f;
  UiUnit unit = UiUnit::Px;
};

struct UiLength2 {
  UiLength x;
  UiLength y;
};

struct UiPadding {
  UiLength left;
  UiLength top;
  UiLength right;
  UiLength bottom;
};

struct UiTextureRegion {
  std::reference_wrapper<const Material> material;
  UvRegion region = {
      .min = glm::vec2(0.0f),
      .max = glm::vec2(1.0f),
      .arrayLayer = 0,
  };

  UiTextureRegion(const Material& material, const UvRegion& region = {}) 
      : material(material), region(region) {}
};

struct UiInsets {
  float left = 0.0f, top = 0.0f, right = 0.0f, bottom = 0.0f;
};

struct UiLayout {
  UiRect rect;
  UiInsets padding;
};

}  // namespace gfx