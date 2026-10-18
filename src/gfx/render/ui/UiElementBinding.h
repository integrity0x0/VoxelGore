#pragma once

#include <string_view>

#include "UiElement.h"

namespace gfx {
class UiElementBinding {
 public:
  UiElementBinding() = delete;

  static constexpr std::string_view kElementMeta = "ui.Element";
  static void Register(const script::LuaState& state);
  static void Push(const script::LuaState& state, UiElement* element);
 private:
};
}  // namespace gfx