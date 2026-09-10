#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "LuaState.h"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

namespace script {

class LuaScript {
 public:
  LuaScript(const LuaState& luaState, const std::vector<char>& bytes, std::string_view chunkName);

  void call(const char* functionName, int nargs = 0, int nresults = 0) const;
  bool hasFunction(const char* functionName) const;
  lua_State* getState() const { return rawState(); }

  LuaScript(const LuaScript&) = delete;
  LuaScript& operator=(const LuaScript&) = delete;
  LuaScript(LuaScript&& other) noexcept;
  LuaScript& operator=(LuaScript&& other) noexcept;
  ~LuaScript();

 private:
  lua_State* rawState() const;

  const LuaState* luaState_;
  int envRef_;
};

}  // namespace script