#pragma once

#include <string_view>

#include "../../../script/LuaState.h"
#include "UiPage.h"

namespace gfx {

class UiPageBinding {
 public:
  UiPageBinding() = delete;

  static constexpr std::string_view kPageMeta = "ui.Page";

  static void Push(const script::LuaState& state, UiPage* page);
  static void Register(const script::LuaState& state);
};

}  // namespace gfx
