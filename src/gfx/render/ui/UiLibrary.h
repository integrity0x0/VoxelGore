#pragma once

#include <functional>

#include "Ui.h"
#include "UiElementBinding.h"
#include "UiLuaTypes.h"
#include "UiPageBinding.h"

namespace gfx {

class UiLibrary {
 public:
  struct Context {
    std::reference_wrapper<const script::LuaState> state;
    std::reference_wrapper<Ui> ui;

    Context(const script::LuaState& state, Ui& ui) : state(state), ui(ui) {}
  };

  explicit UiLibrary(Context ctxt);

 private:
  void SetFunction(const char* name, lua_CFunction function);

  static int LuaPush(lua_State* L);
  static int LuaPop(lua_State* L);
  static int LuaClear(lua_State* L);

  [[nodiscard]] static UiLibrary& GetSelf(lua_State* L);

  Context ctxt_;
};

}  // namespace gfx
