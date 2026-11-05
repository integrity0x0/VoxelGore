#pragma once

#include <cstdint>
#include <utility>
#include <glm/glm.hpp>

#include "../../UvRegion.h"
#include "../../../script/LuaRef.h"
#include "../../common/texture/MaterialManager.h"
#include "../../common/font/FontFamily.h"

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
  MaterialId materialId = kInvalidMaterialId;
  UvRegion region = {
      .min = glm::vec2(0.0f),
      .max = glm::vec2(1.0f),
      .arrayLayer = 0,
  };

  UiTextureRegion(MaterialId materialId, const UvRegion& region = {})
      : materialId(materialId), region(region) {}
};

struct UiInsets {
  float left = 0.0f, top = 0.0f, right = 0.0f, bottom = 0.0f;
};

struct UiLayout {
  UiRect rect;
  UiInsets padding;
};

enum class UiState : uint32_t {
  Disabled,
  Hovered,
  Held,
  Checked,
  Count
};

enum class UiStateFlags : uint32_t {
  None = 0,
  Disabled = 1 << static_cast<uint32_t>(UiState::Disabled),
  Hovered = 1 << static_cast<uint32_t>(UiState::Hovered),
  Held = 1 << static_cast<uint32_t>(UiState::Held),
  Checked = 1 << static_cast<uint32_t>(UiState::Checked),
  All = UINT32_MAX
};

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

enum class UiFillFrom {
  Left, Right, Top, Bottom, CenterX, CenterY, Count
};

struct UiResolvedVisual {
  glm::vec4 color{1.0f};
  glm::vec4 bgColor{0.0f};
  glm::vec4 borderColor{1.0f};
  glm::vec4 fillColor{1.0f};
  glm::vec4 shadowColor{0.0f};

  float radius = 0.0f;
  float bgRadius = 0.0f;
  float fillRadius = 0.0f;
  float borderWidth = 0.0f;
  float scale = 1.0f;
  float fontSize = 16.0f;

  std::optional<UiTextureRegion> src;
  std::optional<UiTextureRegion> bgImage;
  std::optional<UiTextureRegion> borderImage;
  std::optional<UiTextureRegion> fillImage;

  UiFillFrom fillFrom = UiFillFrom::Left;

  std::u32string text;
  FontFamily* font = nullptr;
  FontWeight fontWeight = FontWeight(400);
  bool italic = false;
};

struct UiVisualProps {
  std::optional<glm::vec4> color;
  std::optional<glm::vec4> bgColor;
  std::optional<glm::vec4> borderColor;
  std::optional<glm::vec4> fillColor;
  std::optional<glm::vec4> shadowColor;

  std::optional<float> radius;
  std::optional<float> bgRadius;
  std::optional<float> borderWidth;
  std::optional<float> scale;
  std::optional<float> fillRadius;
  std::optional<float> fontSize;

  std::optional<UiTextureRegion> src;
  std::optional<UiTextureRegion> bgImage;
  std::optional<UiTextureRegion> borderImage;
  std::optional<UiTextureRegion> fillImage;

  std::optional<UiFillFrom> fillFrom;

  std::optional<std::u32string> text;
  FontFamily* font = nullptr;
  std::optional<FontWeight> fontWeight;
  std::optional<bool> italic;

  void MergeFrom(const UiVisualProps& o) {
    if (o.color) color = o.color;
    if (o.bgColor) bgColor = o.bgColor;
    if (o.borderColor) borderColor = o.borderColor;
    if (o.fillColor) fillColor = o.fillColor;
    if (o.shadowColor) shadowColor = o.shadowColor;

    if (o.radius) radius = o.radius;
    if (o.bgRadius) bgRadius = o.bgRadius;
    if (o.borderWidth) borderWidth = o.borderWidth;
    if (o.scale) scale = o.scale;
    if (o.fillRadius) fillRadius = o.fillRadius;
    if (o.fontSize) fontSize = o.fontSize;

    if (o.src) src = o.src;
    if (o.bgImage) bgImage = o.bgImage;
    if (o.borderImage) borderImage = o.borderImage;
    if (o.fillImage) fillImage = o.fillImage;

    if (o.fillFrom) fillFrom = o.fillFrom;

    if (o.text) text = o.text;
    if (o.font) font = o.font;
    if (o.fontWeight) fontWeight = o.fontWeight;
    if (o.italic) italic = o.italic;
  }

  void ApplyTo(UiResolvedVisual& r) const {
    if (color) r.color = *color;
    if (bgColor) r.bgColor = *bgColor;
    if (borderColor) r.borderColor = *borderColor;
    if (fillColor) r.fillColor = *fillColor;
    if (shadowColor) r.shadowColor = *shadowColor;

    if (radius) r.radius = *radius;
    if (bgRadius) r.bgRadius = *bgRadius;
    if (borderWidth) r.borderWidth = *borderWidth;
    if (scale) r.scale = *scale;
    if (fillRadius) r.fillRadius = *fillRadius;
    if (fontSize) r.fontSize = *fontSize;

    if (src) r.src = src;
    if (bgImage) r.bgImage = bgImage;
    if (borderImage) r.borderImage = borderImage;
    if (fillImage) r.fillImage = fillImage;

    if (fillFrom) r.fillFrom = *fillFrom;

    if (text) r.text = *text;
    if (font) r.font = font;
    if (fontWeight) r.fontWeight = *fontWeight;
    if (italic) r.italic = *italic;
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


using UiStateOverrides = std::array<UiVisualProps, static_cast<size_t>(UiState::Count)>;

struct UiStyle {
  UiVisualProps visualProps;
  UiLayoutProps layoutProps;
  UiStateOverrides stateOverrides;

  void MergeFrom(const UiStyle& o) {
    visualProps.MergeFrom(o.visualProps);
    layoutProps.MergeFrom(o.layoutProps);
    for (size_t i = 0; i < stateOverrides.size(); ++i)
      stateOverrides[i].MergeFrom(o.stateOverrides[i]);
  }
};


}  // namespace gfx