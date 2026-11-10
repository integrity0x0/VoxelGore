#pragma once

#include <array>
#include <optional>
#include <string_view>

#include "UiElementType.h"
#include "UiTypes.h"

namespace gfx {
enum class UiProp : uint8_t {
  // layout
  Pos,
  Size,
  Anchor,
  Pivot,
  Padding,

  // visual
  Color,
  BgColor,
  BorderColor,
  Radius,
  BgRadius,
  BorderWidth,
  Scale,
  Src,
  BgImage,
  BorderImage,
  FillFrom,
  FillImage,
  FillColor,
  FillRadius,

  // text
  Font,
  FontSize,
  FontWeight,
  Italic,
  Text,
  TextShadow,
  ShadowColor,
  ShadowOffset,

  // grid
  Columns,
  Cell,
  Gap,
  Inset,

  // selection
  SelectionMin,
  SelectionMax,

  // events
  OnClick,
  OnRelease,
  OnCancel,
  OnEnable,
  OnDisable,
  OnHover,
  OnUnhover,
  OnSet,

  Count
};

inline constexpr size_t kPropCount = static_cast<size_t>(UiProp::Count);

inline constexpr std::array<std::string_view, kPropCount> kPropNames{
    // layout
    "pos",
    "size",
    "anchor",
    "pivot",
    "padding",

    // visual
    "color",
    "bg-color",
    "border-color",
    "radius",
    "bg-radius",
    "border",
    "scale",
    "src",
    "bg-image",
    "border-image",
    "fill-from",
    "fill-image",
    "fill-color",
    "fill-radius",

    // text
    "font",
    "font-size",
    "font-weight",
    "italic",
    "text",
    "text-shadow",
    "shadow-color",
    "shadow-offset",

    // grid
    "columns",
    "cell",
    "gap",
    "inset",

    // selection
    "selection-min",
    "selection-max",

    // events
    "on-click",
    "on-release",
    "on-cancel",
    "on-enable",
    "on-disable",
    "on-hover",
    "on-unhover",
    "on-set",
};

constexpr bool AllPropsNamed() {
  for (auto n : kPropNames)
    if (n.empty()) return false;
  return true;
}
static_assert(AllPropsNamed());
static_assert(kPropNames[static_cast<size_t>(UiProp::Pos)] == "pos");
static_assert(kPropNames[static_cast<size_t>(UiProp::BorderImage)] == "border-image");
static_assert(kPropNames[static_cast<size_t>(UiProp::TextShadow)] == "text-shadow");
static_assert(kPropNames[static_cast<size_t>(UiProp::ShadowColor)] == "shadow-color");
static_assert(kPropNames[static_cast<size_t>(UiProp::ShadowOffset)] == "shadow-offset");
static_assert(kPropNames[static_cast<size_t>(UiProp::SelectionMin)] == "selection-min");
static_assert(kPropNames[static_cast<size_t>(UiProp::SelectionMax)] == "selection-max");
static_assert(kPropNames[static_cast<size_t>(UiProp::OnClick)] == "on-click");
static_assert(kPropNames[static_cast<size_t>(UiProp::OnSet)] == "on-set");

constexpr bool IsEventProp(UiProp p) { return p >= UiProp::OnClick; }

constexpr std::optional<UiEventType> EventOf(UiProp p) {
  switch (p) {
    case UiProp::OnClick:
      return UiEventType::Clicked;
    case UiProp::OnRelease:
      return UiEventType::Released;
    case UiProp::OnCancel:
      return UiEventType::Cancelled;
    case UiProp::OnEnable:
      return UiEventType::Enabled;
    case UiProp::OnDisable:
      return UiEventType::Disabled;
    case UiProp::OnHover:
      return UiEventType::Hovered;
    case UiProp::OnUnhover:
      return UiEventType::Unhovered;
    case UiProp::OnSet:
      return UiEventType::Setted;
    default:
      return std::nullopt;
  }
}

constexpr bool IsPropSupported(UiProp p, UiElementType t) {
  switch (p) {
    case UiProp::Src:
      return t == UiElementType::Image;

    case UiProp::FillFrom:
    case UiProp::FillImage:
    case UiProp::FillColor:
    case UiProp::FillRadius:
      return t == UiElementType::Bar;

    case UiProp::Font:
    case UiProp::FontSize:
    case UiProp::FontWeight:
    case UiProp::Italic:
    case UiProp::Text:
    case UiProp::TextShadow:
    case UiProp::ShadowColor:
    case UiProp::ShadowOffset:
      return t == UiElementType::Text;

    case UiProp::OnSet:
      return t == UiElementType::Checkbox || t == UiElementType::Slider;  // TODO: stick

    case UiProp::Columns:
    case UiProp::Cell:
    case UiProp::Gap:
    case UiProp::Inset:
      return t == UiElementType::Grid;

    case UiProp::SelectionMin:
    case UiProp::SelectionMax:
      return t == UiElementType::Container || t == UiElementType::Grid;

    default:
      return true;
  }
}

constexpr bool SupportsAutoSize(UiElementType t) {
  return t == UiElementType::Text || t == UiElementType::Grid;
}

constexpr bool OwnsChildProp(UiElementType parent, UiProp p) {
  switch (parent) {
    case UiElementType::Grid:
      return p == UiProp::Pos || p == UiProp::Size || p == UiProp::Anchor;
    default:
      return false;
  }
}

}  // namespace gfx