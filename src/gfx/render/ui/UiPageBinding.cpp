#include "UiPageBinding.h"

#include <lua.hpp>

namespace gfx {
namespace {

int AbsIndex(lua_State* L, int index) {
  return index > 0 || index <= LUA_REGISTRYINDEX ? index : lua_gettop(L) + index + 1;
}

UiPage* CheckPage(lua_State* L, int index = 1) {
  auto** userdata =
      static_cast<UiPage**>(luaL_checkudata(L, index, UiPageBinding::kPageMeta.data()));

  luaL_argcheck(L, *userdata != nullptr, index, "UiPage is null");
  return *userdata;
}

void SetFunction(lua_State* L, int table, const char* name, lua_CFunction function) {
  table = AbsIndex(L, table);
  lua_pushcfunction(L, function);
  lua_setfield(L, table, name);
}

int GetId(lua_State* L) {
  const auto id = CheckPage(L)->GetId();
  lua_pushlstring(L, id.data(), id.size());
  return 1;
}

int GetInputMode(lua_State* L) {
  lua_pushinteger(L, static_cast<lua_Integer>(CheckPage(L)->GetInputMode()));
  return 1;
}

int GetType(lua_State* L) {
  lua_pushinteger(L, static_cast<lua_Integer>(CheckPage(L)->GetType()));
  return 1;
}

int HasScript(lua_State* L) {
  lua_pushboolean(L, CheckPage(L)->GetScript().has_value());
  return 1;
}

int SetInputMode(lua_State* L) {
  const auto mode = static_cast<UiInputMode>(luaL_checkinteger(L, 2));
  CheckPage(L)->SetInputMode(mode);
  return 0;
}

int SetType(lua_State* L) {
  const auto type = static_cast<UiPageType>(luaL_checkinteger(L, 2));
  CheckPage(L)->SetType(type);
  return 0;
}

}  // namespace

void UiPageBinding::Push(const script::LuaState& state, UiPage* page) {
  lua_State* L = state.Get();
  if (page == nullptr) {
    lua_pushnil(L);
    return;
  }

  auto** userdata = static_cast<UiPage**>(lua_newuserdata(L, sizeof(UiPage*)));

  *userdata = page;

  luaL_getmetatable(L, UiPageBinding::kPageMeta.data());
  lua_setmetatable(L, -2);
}

void UiPageBinding::Register(const script::LuaState& state) {
  lua_State* L = state.Get();

  if (luaL_newmetatable(L, kPageMeta.data())) {
    // page.__index = page
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    // Getters
    SetFunction(L, -1, "GetId", GetId);
    SetFunction(L, -1, "GetInputMode", GetInputMode);
    SetFunction(L, -1, "GetType", GetType);
    SetFunction(L, -1, "HasScript", HasScript);

    // Setters
    SetFunction(L, -1, "SetInputMode", SetInputMode);
    SetFunction(L, -1, "SetType", SetType);
  }

  lua_pop(L, 1);
}

}  // namespace gfx