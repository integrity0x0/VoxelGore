#pragma once

#include <functional>
#include <utility>

#include "LuaState.h"

namespace script {
class LuaRef {
 public:
  LuaRef(const LuaState& state, int luaRef) : state_(state), luaRef_(luaRef) {}

  LuaRef(const LuaRef&) = delete;
  LuaRef& operator=(const LuaRef&) = delete;

  LuaRef(LuaRef&& other) noexcept
      : state_(other.state_), luaRef_(std::exchange(other.luaRef_, LUA_NOREF)) {}

  LuaRef& operator=(LuaRef&& other) noexcept {
    if (this != &other) {
      Release();
      state_ = other.state_;
      luaRef_ = std::exchange(other.luaRef_, LUA_NOREF);
    }
    return *this;
  }

  ~LuaRef() { Release(); }

  [[nodiscard]] int Get() const { return luaRef_; }

  void Push(lua_State* L) const { lua_rawgeti(L, LUA_REGISTRYINDEX, luaRef_); }

 private:
  void Release() {
    if (luaRef_ == LUA_NOREF) return;
    luaL_unref(state_.get().Get(), LUA_REGISTRYINDEX, luaRef_);
    luaRef_ = LUA_NOREF;
  }

 private:
  std::reference_wrapper<const LuaState> state_;
  int luaRef_ = LUA_NOREF;
};
}  // namespace script