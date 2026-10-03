#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <span>

#include "LuaState.h"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

namespace script {

class LuaScript {
 public:
  LuaScript(const LuaState& luaState, std::span<const std::byte> bytes, std::string_view chunkName);

  void Call(const char* functionName, int nargs = 0, int nresults = 0) const;
  [[nodiscard]] bool HasFunction(const char* functionName) const;
  [[nodiscard]] lua_State* GetState() const { return GetRawState(); }

  LuaScript(const LuaScript&) = delete;
  LuaScript& operator=(const LuaScript&) = delete;
  LuaScript(LuaScript&& other) noexcept;
  LuaScript& operator=(LuaScript&& other) noexcept;
  ~LuaScript();

 private:
  lua_State* GetRawState() const {
    return luaState_.get().Get();
  };

 private:
  std::reference_wrapper<const LuaState> luaState_;
  int envRef_;
};

}  // namespace script