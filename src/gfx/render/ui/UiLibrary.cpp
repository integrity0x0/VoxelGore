
#include "UiLibrary.h"

#include <lauxlib.h>
#include <lua.h>

namespace gfx {

UiLibrary::UiLibrary(Context ctxt) : ctxt_(ctxt) {
  const auto& state = ctxt_.state.get();

  UiLuaTypes::Register(state);
  UiElementBinding::Register(state);
  UiPageBinding::Register(state);

  SetFunction("push", &UiLibrary::LuaPush);
  SetFunction("pop", &UiLibrary::LuaPop);
  SetFunction("clear", &UiLibrary::LuaClear);
}

void UiLibrary::SetFunction(const char* name, lua_CFunction function) {
  lua_State* L = ctxt_.state.get().Get();

  lua_getglobal(L, "ui");

  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);

    lua_pushvalue(L, -1);
    lua_setglobal(L, "ui");
  }

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, function, 1);

  // ui[name] = closure
  lua_setfield(L, -2, name);

  lua_pop(L, 1);
}

UiLibrary& UiLibrary::GetSelf(lua_State* L) {
  auto* self = static_cast<UiLibrary*>(lua_touserdata(L, lua_upvalueindex(1)));
  return *self;
}

int UiLibrary::LuaPush(lua_State* L) {
  UiLibrary& self = GetSelf(L);

  size_t length = 0;
  const char* path = luaL_checklstring(L, 1, &length);

  self.ctxt_.ui.get().Push(std::string_view(path, length));
  return 0;
}

int UiLibrary::LuaPop(lua_State* L) {
  UiLibrary& self = GetSelf(L);

  self.ctxt_.ui.get().Pop();
  return 0;
}

int UiLibrary::LuaClear(lua_State* L) {
  UiLibrary& self = GetSelf(L);

  self.ctxt_.ui.get().Clear();
  return 0;
}

}  // namespace gfx
