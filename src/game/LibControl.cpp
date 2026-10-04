#include "LibControl.h"

#include <lua.hpp>

namespace gm {

namespace {

static ControlState* ctx(lua_State* L) {
  return static_cast<ControlState*>(lua_touserdata(L, lua_upvalueindex(1)));
}

static int lua_setMoveLeft(lua_State* L) {
  ctx(L)->setMoveLeft(lua_toboolean(L, 1));
  return 0;
}

static int lua_setMoveRight(lua_State* L) {
  ctx(L)->setMoveRight(lua_toboolean(L, 1));
  return 0;
}

static int lua_setMoveUp(lua_State* L) {
  ctx(L)->setMoveUp(lua_toboolean(L, 1));
  return 0;
}

static int lua_setMoveDown(lua_State* L) {
  ctx(L)->setMoveDown(lua_toboolean(L, 1));
  return 0;
}

static int lua_setMoveForward(lua_State* L) {
  ctx(L)->setMoveForward(lua_toboolean(L, 1));
  return 0;
}

static int lua_setMoveBack(lua_State* L) {
  ctx(L)->setMoveBack(lua_toboolean(L, 1));
  return 0;
}

static int lua_setJump(lua_State* L) {
  ctx(L)->setJump(lua_toboolean(L, 1));
  return 0;
}

static int lua_setSneak(lua_State* L) {
  ctx(L)->setSneak(lua_toboolean(L, 1));
  return 0;
}

static int lua_setSprint(lua_State* L) {
  ctx(L)->setSprint(lua_toboolean(L, 1));
  return 0;
}

static int lua_setBreak(lua_State* L) {
  ctx(L)->setBreak(lua_toboolean(L, 1));
  return 0;
}

static int lua_setInteract(lua_State* L) {
  ctx(L)->setInteract(lua_toboolean(L, 1));
  return 0;
}

static int lua_setDrop(lua_State* L) {
  ctx(L)->setDrop(lua_toboolean(L, 1));
  return 0;
}

static int lua_setHotbarSlot(lua_State* L) {
  ctx(L)->setHotbarSlot(static_cast<uint32_t>(luaL_checkinteger(L, 1)));
  return 0;
}

static int lua_clear(lua_State* L) {
  ctx(L)->clear();
  return 0;
}

}  // namespace

const LibControl::FnEntry LibControl::kControlFunctions[] = {
    {"setMoveLeft", lua_setMoveLeft},
    {"setMoveRight", lua_setMoveRight},
    {"setMoveUp", lua_setMoveUp},
    {"setMoveDown", lua_setMoveDown},
    {"setMoveForward", lua_setMoveForward},
    {"setMoveBack", lua_setMoveBack},
    {"setJump", lua_setJump},
    {"setSneak", lua_setSneak},
    {"setSprint", lua_setSprint},
    {"setBreak", lua_setBreak},
    {"setInteract", lua_setInteract},
    {"setDrop", lua_setDrop},
    {"setHotbarSlot", lua_setHotbarSlot},
    {"clear", lua_clear},
};

LibControl::LibControl(ControlState& state, const script::LuaState& luaState)
    : state_(&state), L_(luaState.get()) {
  lua_newtable(L_);
  registerClosures(kControlFunctions, std::size(kControlFunctions));
  lua_setglobal(L_, "control");
}

void LibControl::registerClosures(const FnEntry* entries, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    lua_pushlightuserdata(L_, state_);
    lua_pushcclosure(L_, entries[i].second, 1);
    lua_setfield(L_, -2, entries[i].first);
  }
}

}  // namespace gm