#include "LuaState.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include <stdexcept>
#include <string>

namespace script {

void LuaStateDeleter::operator()(lua_State* state) const { lua_close(state); }

#ifdef __ANDROID__
static int lua_print_to_logcat(lua_State* L) {
  int nargs = lua_gettop(L);
  std::string output;

  for (int i = 1; i <= nargs; ++i) {
    if (i > 1) output += "\t";

    if (luaL_callmeta(L, i, "__tostring")) {
      output += lua_tostring(L, -1);
      lua_pop(L, 1);
      continue;
    }

    int type = lua_type(L, i);
    switch (type) {
      case LUA_TNIL:
        output += "nil";
        break;
      case LUA_TBOOLEAN:
        output += lua_toboolean(L, i) ? "true" : "false";
        break;
      case LUA_TSTRING:
      case LUA_TNUMBER: {
        const char* s = lua_tostring(L, i);
        output += (s != nullptr ? s : "");
        break;
      }
      default: {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s: %p", lua_typename(L, type), lua_topointer(L, i));
        output += buf;
        break;
      }
    }
  }

  __android_log_print(ANDROID_LOG_INFO, "VoxelGore", "%s", output.c_str());
  return 0;
}
#endif

LuaState::LuaState() : state_(luaL_newstate()) {
  if (!state_) {
    throw std::runtime_error("failed to create lua_State");
  }
  luaL_openlibs(state_.get());
#ifdef __ANDROID__
  lua_pushcfunction(state_.get(), lua_print_to_logcat);
#endif
  lua_setglobal(state_.get(), "print");
}

lua_State* LuaState::get() const { return state_.get(); }

}  // namespace script