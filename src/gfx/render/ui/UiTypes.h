#pragma once

#include <cstdint>
#include <utility>
#include <glm/glm.hpp>

#include "../../UvRegion.h"
#include "../../../script/LuaRef.h"
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

enum class UiStateFlags {
  Disabled = 1 << 0,
  Hovered = 1 << 1,
  Held = 1 << 2,
  Checked = 1 << 3,
};

static constexpr size_t kUiStateCount = 4;

constexpr UiStateFlags operator&(UiStateFlags first, UiStateFlags second) {
  return static_cast<UiStateFlags>(static_cast<uint32_t>(first) & static_cast<uint32_t>(second));
}

constexpr UiStateFlags operator|(UiStateFlags first, UiStateFlags second) {
  return static_cast<UiStateFlags>(static_cast<uint32_t>(first) | static_cast<uint32_t>(second));
}

constexpr UiStateFlags operator~(UiStateFlags first) {
  return static_cast<UiStateFlags>(~static_cast<uint32_t>(first));
}

constexpr UiStateFlags& operator|=(UiStateFlags& first, UiStateFlags second) {
  return first = first | second;
}

constexpr UiStateFlags& operator&=(UiStateFlags& first, UiStateFlags second) {
  return first = first & second;
}

struct UiStateOverrides {
  std::optional<UiPadding> padding;
  std::optional<glm::vec4> bgColor;
  std::optional<UiTextureRegion> bgImage;
  std::optional<UiTextureRegion> image;
  std::optional<glm::vec4> color;

  std::optional<std::string> text;
  std::optional<float> scale;
};

struct UiCallback {
  std::string functionName;
  std::optional<script::LuaRef> ref;
  bool dead = false;
};

enum class UiEventType {
  Clicked,
  Released,
  Hovered,
  Unhovered,
  Cancelled,
  Disabled,
  Enabled,
  Setted,
  Count
};

struct UiVisualProps {
  std::optional<glm::vec4> color, bgColor;
  std::optional<float> radius, bgRadius, scale;
  std::optional<UiTextureRegion> image, bgImage;
  void MergeFrom(const UiVisualProps& o) {
    if (o.color) color = o.color;
    if (o.bgColor) bgColor = o.bgColor;
    if (o.radius) radius = o.radius;
    if (o.bgRadius) bgRadius = o.bgRadius;
    if (o.scale) scale = o.scale;
    if (o.image) image = o.image;
    if (o.bgImage) bgImage = o.bgImage;
  }
};

struct UiLayoutProps {
  std::optional<UiLength2> pos, size;
  std::optional<UiPoint> anchor, pivot;
  std::optional<UiPadding> padding;
  void MergeFrom(const UiLayoutProps& o) {
    if (o.pos) pos = o.pos;
    if (o.size) size = o.size;
    if (o.anchor) anchor = o.anchor;
    if (o.pivot) pivot = o.pivot;
    if (o.padding) padding = o.padding;
  }
};

struct UiStyle {
  UiVisualProps visualProps;
  UiLayoutProps layoutProps;
  std::array<UiVisualProps, kUiStateCount> stateOverrides;
};


}  // namespace gfx