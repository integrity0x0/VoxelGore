#pragma once

#include "UiTypes.h"
#include "UiPage.h"
#include "../../../script/LuaState.h"

namespace gfx {
class UiLuaTypes {
 public:
  UiLuaTypes() = delete;

  static void Register(const script::LuaState& state);
};
}  // namespace gfx
