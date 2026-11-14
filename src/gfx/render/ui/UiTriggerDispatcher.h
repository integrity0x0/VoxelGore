#pragma once

#include <functional>

#include "UiElement.h"
#include "UiPage.h"
#include "UiEvents.h"
#include "../../../script/LuaState.h"

namespace gfx {
class UiTriggerDispatcher {
 public:
  UiTriggerDispatcher(const script::LuaState& luaState) : state_(luaState) {}

  void Dispatch(UiEventQueue& events) const;
  void DispatchPageEvent(UiPage& page, UiPageEvent type) const;
 private:
  void Fire(const UiCallback& callback, int nargs) const;

 private:
  std::reference_wrapper<const script::LuaState> state_;
};
}  // namespace gfx