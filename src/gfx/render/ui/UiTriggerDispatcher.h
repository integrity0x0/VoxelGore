#pragma once

#include <functional>

#include "UiElement.h"
#include "UiEvents.h"
#include "../../../script/LuaState.h"

namespace gfx {
class UiTriggerDispatcher {
 public:
  UiTriggerDispatcher(const script::LuaState& luaState) : state_(luaState) {}

  void Dispatch(UiEventQueue& events) const;

 private:
  std::reference_wrapper<const script::LuaState> state_;
};
}  // namespace gfx