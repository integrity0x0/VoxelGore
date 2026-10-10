#pragma once

#include <functional>

#include "../../../script/LuaState.h"

class UiTriggerDispatcher {
 public:
  UiTriggerDispatcher(const script::LuaState& luaState)
      : luaState_(luaState) {}
 private:
  std::reference_wrapper<const script::LuaState> luaState_;
};
