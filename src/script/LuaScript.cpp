#include "LuaScript.h"

namespace script {

LuaScript::LuaScript(const LuaState& luaState, std::span<const std::byte> bytes,
                     std::string_view chunkName)
    : luaState_(luaState), envRef_(LUA_NOREF) {
  lua_State* L = GetRawState();

  if (luaL_loadbuffer(L, reinterpret_cast<const char*>(bytes.data()), bytes.size(), chunkName.data()) != LUA_OK) {
    std::string error = lua_tostring(L, -1);
    lua_pop(L, 1);
    throw std::runtime_error("Lua compile error [" + std::string(chunkName) + "]: " + error);
  }

  lua_newtable(L);
  lua_newtable(L);
  lua_getglobal(L, "_G");
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);

  lua_pushvalue(L, -1);
  envRef_ = luaL_ref(L, LUA_REGISTRYINDEX);

  lua_setfenv(L, -2);

  if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
    std::string error = lua_tostring(L, -1);
    lua_pop(L, 1);
    luaL_unref(L, LUA_REGISTRYINDEX, envRef_);
    envRef_ = LUA_NOREF;
    throw std::runtime_error("Lua runtime error [" + std::string(chunkName) + "]: " + error);
  }
}

void LuaScript::Call(const char* functionName, int nargs, int nresults) const {
  lua_State* L = GetRawState();

  lua_rawgeti(L, LUA_REGISTRYINDEX, envRef_);
  lua_getfield(L, -1, functionName);

  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    throw std::runtime_error(std::string("Lua function not found: ") + functionName);
  }

  lua_remove(L, -2);

  if (nargs > 0) {
    lua_insert(L, -(nargs + 1));
  }

  if (lua_pcall(L, nargs, nresults, 0) != LUA_OK) {
    std::string error = lua_tostring(L, -1);
    lua_pop(L, 1);
    throw std::runtime_error(std::string("Lua Call error in ") + functionName + ": " + error);
  }
}

bool LuaScript::HasFunction(const char* functionName) const {
  lua_State* L = GetRawState();
  lua_rawgeti(L, LUA_REGISTRYINDEX, envRef_);
  lua_getfield(L, -1, functionName);
  bool result = lua_isfunction(L, -1);
  lua_pop(L, 2);
  return result;
}

LuaScript::LuaScript(LuaScript&& other) noexcept
    : luaState_(other.luaState_), envRef_(other.envRef_) {
  other.envRef_ = LUA_NOREF;
}

LuaScript& LuaScript::operator=(LuaScript&& other) noexcept {
  if (this != &other) {
    if (envRef_ != LUA_NOREF) {
      luaL_unref(GetRawState(), LUA_REGISTRYINDEX, envRef_);
    }
    luaState_ = other.luaState_;
    envRef_ = other.envRef_;
    other.envRef_ = LUA_NOREF;
  }
  return *this;
}

LuaScript::~LuaScript() {
  if (envRef_ != LUA_NOREF) {
    luaL_unref(GetRawState(), LUA_REGISTRYINDEX, envRef_);
  }
}

}  // namespace script