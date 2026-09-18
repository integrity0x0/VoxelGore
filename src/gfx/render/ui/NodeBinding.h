#pragma once

#include <lua.hpp>
#include <memory>

#include "Node.h"

namespace gfx::ui {
void registerNodeMetatable(lua_State* L);

void pushNode(lua_State* L, std::shared_ptr<Node> node);

std::shared_ptr<Node>& checkNode(lua_State* L, int idx);
}  // namespace gfx::ui