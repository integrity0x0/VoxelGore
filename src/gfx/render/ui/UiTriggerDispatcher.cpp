#include "UiTriggerDispatcher.h"

#include "Ui.h"
#include "UiElementBinding.h"

namespace gfx {
void UiTriggerDispatcher::Dispatch(UiEventQueue& events) const {
  lua_State* L = state_.get().Get();

  while (!events.empty()) {
    const UiEvent event = events.front();
    events.pop();

    UiElement& element = event.element.get();
    UiCallback& callback = element.GetCallback(event.type);
    if (!callback.ref) continue;

    const int top = lua_gettop(L);

    callback.ref->Push(L);                   // [fn]
    UiElementBinding::Push(state_.get(), &element);  // [fn, handle]

    if (lua_pcall(L, 1, 0, 0) != 0) {
      const char* message = lua_tostring(L, -1);
      std::cout << message << "\n";
      // TODO logger: callback.functionName, message ? message : "non-string error"
    }

    lua_settop(L, top);
  }
}
}  // namespace gfx