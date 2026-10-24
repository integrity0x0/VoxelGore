#pragma once
#include <array>
#include <optional>
#include <string_view>

#include "UiTypes.h"
#include "UiElement.h"

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
    "pos",         "size",         "anchor",       "pivot",     "padding",    "color",
    "bg-color",    "border-color", "radius",       "bg-radius", "border",     "scale",
    "src",         "bg-image",     "border-image", "fill-from", "fill-image", "fill-color",
    "fill-radius", "on-click",     "on-release",   "on-cancel", "on-enable",  "on-disable",
    "on-hover",    "on-unhover",   "on-set"};

constexpr bool AllPropsNamed() {
  for (auto n : kPropNames)
    if (n.empty()) return false;
  return true;
}
static_assert(AllPropsNamed());
static_assert(kPropNames[static_cast<size_t>(UiProp::Pos)] == "pos");
static_assert(kPropNames[static_cast<size_t>(UiProp::BorderImage)] == "border-image");
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
    case UiProp::OnSet:
      return t == UiElementType::Checkbox || t == UiElementType::Slider;  // + Joystick
    default:
      return true;
  }
}
}  // namespace gfx