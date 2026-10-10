#pragma once

#include <utility>
	
#include "LuaState.h"

namespace script {
class LuaRef {
 public:
  LuaRef(const LuaState& state, int luaRef) 
		: state_(state), luaRef_(luaRef) {}

  LuaRef(LuaRef&& other) noexcept
      : state_(other.state_), luaRef_(std::exchange(other.luaRef_, LUA_NOREF)) {}

  int Get() const { return luaRef_; }

  ~LuaRef() { 
    if (luaRef_ == LUA_NOREF) return;
    lua_pushinteger(state_.get().get(), luaRef_);
    luaL_unref(state_.get().get(), -1, luaRef_);
  }
 private:
  std::reference_wrapper<const LuaState> state_;
  int luaRef_ = LUA_NOREF;
};
}  // namespace script