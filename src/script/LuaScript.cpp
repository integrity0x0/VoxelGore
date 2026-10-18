#include "LuaScript.h"

namespace script {
namespace {
LuaRef CreateEnv(const LuaState& luaState) {
  lua_State* L = luaState.Get();

  lua_newtable(L);                 // [env]
  lua_newtable(L);                 // [env, mt]
  lua_getglobal(L, "_G");          // [env, mt, _G]
  lua_setfield(L, -2, "__index");  // mt.__index = _G; [env, mt]
  lua_setmetatable(L, -2);         // setmetatable(env, mt); [env]

  return LuaRef(luaState, luaL_ref(L, LUA_REGISTRYINDEX));  // luaL_ref снимает env со стека
}

std::string PopErrorMessage(lua_State* L) {
  const char* message = lua_tostring(L, -1);
  std::string result = message ? message : "(non-string error)";
  lua_pop(L, 1);
  return result;
}
}  // namespace

LuaScript::LuaScript(const LuaState& luaState, std::span<const std::byte> bytes,
                     std::string_view chunkName)
    : luaState_(luaState), envRef_(CreateEnv(luaState)) {
  lua_State* L = GetRawState();

  const std::string name(chunkName);

  if (luaL_loadbuffer(L, reinterpret_cast<const char*>(bytes.data()), bytes.size(), name.c_str()) !=
      LUA_OK) {
    throw std::runtime_error("Lua compile error [" + name + "]: " + PopErrorMessage(L));
  }

  lua_rawgeti(L, LUA_REGISTRYINDEX, envRef_.Get());  // [chunk, env]
  lua_setfenv(L, -2);                                // [chunk]

  if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
    throw std::runtime_error("Lua runtime error [" + name + "]: " + PopErrorMessage(L));
  }
}

bool LuaScript::PushFunction(lua_State* L, const char* functionName) const {
  lua_rawgeti(L, LUA_REGISTRYINDEX, envRef_.Get());  // [env]
  lua_pushstring(L, functionName);                   // [env, name]
  lua_rawget(L, -2);                                 // [env, value]: env[name] без __index

  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);  // []
    return false;
  }
  lua_remove(L, -2);  // [fn]
  return true;
}

void LuaScript::Call(const char* functionName, int nargs, int nresults) const {
  lua_State* L = GetRawState();

  if (!PushFunction(L, functionName)) {
    lua_pop(L, nargs);
    throw std::runtime_error(std::string("Lua function not found: ") + functionName);
  }

  if (nargs > 0) {
    lua_insert(L, -(nargs + 1));  // [args..., fn] -> [fn, args...]
  }

  if (lua_pcall(L, nargs, nresults, 0) != LUA_OK) {
    throw std::runtime_error(std::string("Lua Call error in ") + functionName + ": " +
                             PopErrorMessage(L));
  }
}

bool LuaScript::HasFunction(std::string_view functionName) const {
  lua_State* L = GetRawState();
  if (!PushFunction(L, functionName.data())) return false;
  lua_pop(L, 1);
  return true;
}

std::optional<LuaRef> LuaScript::FindFunction(std::string_view functionName) const {
  lua_State* L = GetRawState();
  if (!PushFunction(L, functionName.data())) return std::nullopt;

  const int key = luaL_ref(L, LUA_REGISTRYINDEX);
  return LuaRef(luaState_.get(), key);
}

}  // namespace script