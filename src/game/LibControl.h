#pragma once

#include <cstddef>
#include <utility>

#include "../script/LuaState.h"
#include "ControlState.h"

struct lua_State;

namespace gm {

class LibControl {
 public:
  using LuaCFunction = int (*)(lua_State*);
  using FnEntry = std::pair<const char*, LuaCFunction>;

  LibControl(ControlState& state, const script::LuaState& luaState);

 private:
  static const FnEntry kControlFunctions[];

  void registerClosures(const FnEntry* entries, size_t count);

  ControlState* state_;
  lua_State* L_;
};

}  // namespace gm