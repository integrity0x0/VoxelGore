#pragma once

#include <cstddef>
#include <iterator>
#include <utility>

#include "../../script/LuaState.h"
#include "NodeBinding.h"
#include "Ui.h"

namespace gfx::ui {

class LibGui {
 public:
  LibGui(Ui& ui, const script::LuaState& luaState);

  LibGui(const LibGui&) = delete;
  LibGui& operator=(const LibGui&) = delete;

 private:
  using FnEntry = std::pair<const char*, lua_CFunction>;

  Ui* ui_;
  lua_State* L_;

  void registerClosures(const FnEntry* entries, size_t count);

  static Ui* ctx(lua_State* L);

  static int lua_pushPage(lua_State* L);
  static int lua_popPage(lua_State* L);

  static const FnEntry kGuiFunctions[];
};

}  // namespace gfx::ui