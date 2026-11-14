#include "UiTriggerDispatcher.h"

#include "Ui.h"
#include "UiElementBinding.h"

namespace gfx {
void UiTriggerDispatcher::Fire(const UiCallback& callback, int nargs) const {
  lua_State* L = state_.get().Get();
  if (lua_pcall(L, nargs, 0, 0) != 0) {
    const char* message = lua_tostring(L, -1);
    std::cout << callback.functionName << ": " << (message ? message : "non-string error") << "\n";
  }
}

void UiTriggerDispatcher::Dispatch(UiEventQueue& events) const {
  lua_State* L = state_.get().Get();
  while (!events.empty()) {
    const UiEvent event = events.front();
    events.pop();

    UiElement& element = event.element.get();
    UiCallback& callback = element.GetCallback(event.type);
    if (!callback.ref) continue;

    const int top = lua_gettop(L);
    callback.ref->Push(L);
    UiElementBinding::Push(state_.get(), &element);
    Fire(callback, 1);
    lua_settop(L, top);
  }
}

void UiTriggerDispatcher::DispatchPageEvent(UiPage& page, UiPageEvent type) const {
  UiCallback& callback = page.GetCallback(type);
  if (!callback.ref) return;

  lua_State* L = state_.get().Get();
  const int top = lua_gettop(L);
  callback.ref->Push(L);
  Fire(callback, 0);
  lua_settop(L, top);
}
}  // namespace gfx