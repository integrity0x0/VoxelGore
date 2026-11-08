#include "LibInventory.h"

#include <iterator>
#include <lua.hpp>

namespace gm {

namespace {

static LibInventory::Context* ctx(lua_State* L) {
  return static_cast<LibInventory::Context*>(lua_touserdata(L, lua_upvalueindex(1)));
}

static size_t checkSlot(lua_State* L, int arg) {
  const lua_Integer slot = luaL_checkinteger(L, arg);
  luaL_argcheck(L, slot >= 1 && slot <= static_cast<lua_Integer>(Hotbar::kSize), arg,
                "hotbar slot out of range");
  return static_cast<size_t>(slot - 1);
}

static int lua_getHotbarBlock(lua_State* L) {
  lua_pushinteger(L, static_cast<lua_Integer>(ctx(L)->hotbar->GetBlock(checkSlot(L, 1))));
  return 1;
}

static int lua_setHotbarBlock(lua_State* L) {
  LibInventory::Context* c = ctx(L);
  const size_t slot = checkSlot(L, 1);
  const lua_Integer blockId = luaL_checkinteger(L, 2);
  luaL_argcheck(L, blockId >= 0 && blockId < static_cast<lua_Integer>(c->blocks->GetBlockCount()),
                2, "unknown block id");
  c->hotbar->SetBlock(slot, static_cast<uint32_t>(blockId));
  return 0;
}

static int lua_select(lua_State* L) {
  ctx(L)->hotbar->Select(checkSlot(L, 1));
  return 0;
}

static int lua_getSelected(lua_State* L) {
  lua_pushinteger(L, static_cast<lua_Integer>(ctx(L)->hotbar->GetSelected()) + 1);
  return 1;
}

static int lua_blockCount(lua_State* L) {
  const size_t total = ctx(L)->blocks->GetBlockCount();
  lua_pushinteger(L, static_cast<lua_Integer>(total > 0 ? total - 1 : 0));
  return 1;
}

}  // namespace

const LibInventory::FnEntry LibInventory::kInventoryFunctions[] = {
    {"GetHotbarBlock", lua_getHotbarBlock},
    {"SetHotbarBlock", lua_setHotbarBlock},
    {"Select", lua_select},
    {"GetSelected", lua_getSelected},
    {"BlockCount", lua_blockCount},
};

LibInventory::LibInventory(Hotbar& hotbar, const BlockManager& blocks,
                           const script::LuaState& luaState)
    : ctx_{&hotbar, &blocks}, L_(luaState.Get()) {
  lua_newtable(L_);
  registerClosures(kInventoryFunctions, std::size(kInventoryFunctions));
  lua_setglobal(L_, "inventory");
}

void LibInventory::registerClosures(const FnEntry* entries, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    lua_pushlightuserdata(L_, &ctx_);
    lua_pushcclosure(L_, entries[i].second, 1);
    lua_setfield(L_, -2, entries[i].first);
  }
}

}  // namespace gm