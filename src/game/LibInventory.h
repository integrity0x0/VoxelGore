#pragma once

#include <cstddef>
#include <utility>

#include "../script/LuaState.h"
#include "Hotbar.h"
#include "voxel/BlockManager.h"

struct lua_State;

namespace gm {

class LibInventory {
 public:
  using LuaCFunction = int (*)(lua_State*);
  using FnEntry = std::pair<const char*, LuaCFunction>;

  struct Context {
    Hotbar* hotbar;
    const BlockManager* blocks;
  };

  LibInventory(Hotbar& hotbar, const BlockManager& blocks, const script::LuaState& luaState);

  LibInventory(const LibInventory&) = delete;
  LibInventory& operator=(const LibInventory&) = delete;

 private:
  static const FnEntry kInventoryFunctions[];

  void registerClosures(const FnEntry* entries, size_t count);

  Context ctx_;
  lua_State* L_;
};

}  // namespace gm