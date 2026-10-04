#pragma once

#include <memory>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

namespace script {

struct LuaStateDeleter {
  void operator()(lua_State* state) const;
};

class LuaState {
 public:
  LuaState();

  lua_State* get() const;

 private:
  std::unique_ptr<lua_State, LuaStateDeleter> state_;
};

}  // namespace script