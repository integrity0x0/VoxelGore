#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

#include "LuaRef.h"
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
  [[nodiscard]] bool HasFunction(std::string_view functionName) const;

  [[nodiscard]] std::optional<LuaRef> FindFunction(std::string_view functionName) const;

  [[nodiscard]] lua_State* GetState() const { return GetRawState(); }

 private:
  [[nodiscard]] lua_State* GetRawState() const { return luaState_.get().Get(); }
 
  bool PushFunction(lua_State* L, const char* functionName) const;

 private:
  std::reference_wrapper<const LuaState> luaState_;
  LuaRef envRef_;
};

}  // namespace script