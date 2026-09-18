#include "LibGui.h"

namespace gfx::ui {

const LibGui::FnEntry LibGui::kGuiFunctions[] = {
    {"pushPage", lua_pushPage},
    {"popPage", lua_popPage},
};

LibGui::LibGui(Ui& ui, const script::LuaState& luaState) : ui_(&ui), L_(luaState.get()) {
  lua_newtable(L_);
  registerClosures(kGuiFunctions, std::size(kGuiFunctions));
  lua_setglobal(L_, "gui");

  registerNodeMetatable(L_);
}

void LibGui::registerClosures(const FnEntry* entries, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    lua_pushlightuserdata(L_, ui_);
    lua_pushcclosure(L_, entries[i].second, 1);
    lua_setfield(L_, -2, entries[i].first);
  }
}

Ui* LibGui::ctx(lua_State* L) { return static_cast<Ui*>(lua_touserdata(L, lua_upvalueindex(1))); }

int LibGui::lua_pushPage(lua_State* L) {
  ctx(L)->push(luaL_checkstring(L, 1));
  return 0;
}

int LibGui::lua_popPage(lua_State* L) {
  ctx(L)->pop();
  return 0;
}

}  // namespace gfx::ui