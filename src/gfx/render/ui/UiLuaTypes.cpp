#include "UiLuaTypes.h"

#include <lua.hpp>

namespace gfx {
namespace {

void SetFunction(lua_State* L, int table, const char* name, lua_CFunction function) {
  lua_pushcfunction(L, function);
  lua_setfield(L, table, name);
}

int AbsIndex(lua_State* L, int index) {
  return index > 0 || index <= LUA_REGISTRYINDEX ? index : lua_gettop(L) + index + 1;
}

// UiLength

void PushLength(lua_State* L, UiLength length) {
  lua_createtable(L, 0, 2);

  lua_pushnumber(L, length.value);
  lua_setfield(L, -2, "value");

  lua_pushinteger(L, static_cast<lua_Integer>(length.unit));
  lua_setfield(L, -2, "unit");
}

UiLength CheckLength(lua_State* L, int index) {
  if (lua_type(L, index) == LUA_TNUMBER) {
    return {
        .value = static_cast<float>(lua_tonumber(L, index)),
        .unit = UiUnit::Px,
    };
  }

  luaL_checktype(L, index, LUA_TTABLE);

  index = AbsIndex(L, index);

  lua_getfield(L, index, "value");
  const float value = static_cast<float>(luaL_checknumber(L, -1));
  lua_pop(L, 1);

  lua_getfield(L, index, "unit");
  const auto unit = static_cast<UiUnit>(luaL_checkinteger(L, -1));
  lua_pop(L, 1);

  luaL_argcheck(L, unit == UiUnit::Px || unit == UiUnit::Percent, index, "invalid UI length unit");

  return {.value = value, .unit = unit};
}

int Px(lua_State* L) {
  PushLength(L, {
                    .value = static_cast<float>(luaL_checknumber(L, 1)),
                    .unit = UiUnit::Px,
                });
  return 1;
}

int Percent(lua_State* L) {
  PushLength(L, {
                    .value = static_cast<float>(luaL_checknumber(L, 1)),
                    .unit = UiUnit::Percent,
                });
  return 1;
}

// UiPadding

int Padding(lua_State* L) {
  const int count = lua_gettop(L);

  luaL_argcheck(L, count == 1 || count == 2 || count == 4, 0, "expected 1, 2 or 4 arguments");

  UiLength top;
  UiLength right;
  UiLength bottom;
  UiLength left;

  if (count == 1) {
    top = right = bottom = left = CheckLength(L, 1);
  } else if (count == 2) {
    top = bottom = CheckLength(L, 1);
    left = right = CheckLength(L, 2);
  } else {
    top = CheckLength(L, 1);
    right = CheckLength(L, 2);
    bottom = CheckLength(L, 3);
    left = CheckLength(L, 4);
  }

  lua_createtable(L, 0, 4);

  PushLength(L, left);
  lua_setfield(L, -2, "left");

  PushLength(L, top);
  lua_setfield(L, -2, "top");

  PushLength(L, right);
  lua_setfield(L, -2, "right");

  PushLength(L, bottom);
  lua_setfield(L, -2, "bottom");

  return 1;
}

// Enum tables

template <typename T>
void SetEnumValue(lua_State* L, int table, const char* name, T value) {
  lua_pushinteger(L, static_cast<lua_Integer>(value));
  lua_setfield(L, table, name);
}

void RegisterAlignX(lua_State* L, int ui) {
  lua_createtable(L, 0, 3);
  const int table = AbsIndex(L, -1);

  SetEnumValue(L, table, "Left", UiAlignX::Left);
  SetEnumValue(L, table, "Center", UiAlignX::Center);
  SetEnumValue(L, table, "Right", UiAlignX::Right);

  lua_setfield(L, ui, "AlignX");
}

void RegisterAlignY(lua_State* L, int ui) {
  lua_createtable(L, 0, 3);
  const int table = AbsIndex(L, -1);

  SetEnumValue(L, table, "Top", UiAlignY::Top);
  SetEnumValue(L, table, "Center", UiAlignY::Center);
  SetEnumValue(L, table, "Bottom", UiAlignY::Bottom);

  lua_setfield(L, ui, "AlignY");
}

void RegisterUnit(lua_State* L, int ui) {
  lua_createtable(L, 0, 2);
  const int table = AbsIndex(L, -1);

  SetEnumValue(L, table, "Px", UiUnit::Px);
  SetEnumValue(L, table, "Percent", UiUnit::Percent);

  lua_setfield(L, ui, "Unit");
}

// Page props

void RegisterInputMode(lua_State* L, int ui) {
  lua_createtable(L, 0, 1);
  const int table = AbsIndex(L, -1);

  SetEnumValue(L, table, "Passthrough", UiInputMode::Passthrough);
  SetEnumValue(L, table, "Capture", UiInputMode::Capture);
  
  lua_setfield(L, ui, "InputMode");
}

void RegisterPageType(lua_State* L, int ui) {
  lua_createtable(L, 0, 1);
  const int table = AbsIndex(L, -1);

  SetEnumValue(L, ui, "Overlay", UiPageType::Overlay);
  SetEnumValue(L, ui, "Screen", UiPageType::Screen);

  lua_setfield(L, ui, "PageType");
}
}  // namespace

void UiLuaTypes::Register(const script::LuaState& state) {
  lua_State* L = state.Get();

  lua_getglobal(L, "ui");

  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "ui");
  }

  const int ui = AbsIndex(L, -1);

  SetFunction(L, ui, "Px", Px);
  SetFunction(L, ui, "Percent", Percent);
  SetFunction(L, ui, "Padding", Padding);

  RegisterAlignX(L, ui);
  RegisterAlignY(L, ui);
  RegisterUnit(L, ui);

  RegisterInputMode(L, ui);
  RegisterPageType(L, ui);

  lua_pop(L, 1);
}

}  // namespace gfx