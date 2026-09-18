#pragma once

#include <memory>
#include <vector>

#include "../../../script/LuaScript.h"
#include "Node.h"
#include "NodeBinding.h"

namespace gfx::ui {
class TriggerDispatcher {
 public:
  void dispatch(const std::vector<std::shared_ptr<Node>>& roots, const script::LuaScript& script) {
    for (const auto& node : roots) {
      dispatchNode(node, script);
    }
  }

 private:
  void dispatchNode(const std::shared_ptr<Node>& node, const script::LuaScript& script) {
    bool justPressed = node->consumeJustPressed();
    bool justReleased = node->consumeJustReleased();

    fire(node, script, node->getOnPressLua(), justPressed);
    fire(node, script, node->getOnHeldLua(), node->isPressed());
    fire(node, script, node->getOnReleaseLua(), justReleased);

    for (const auto& child : node->getChildren()) {
      dispatchNode(child, script);
    }
  }

  void fire(const std::shared_ptr<Node>& node, const script::LuaScript& script,
            const std::string& functionName, bool shouldFire) {
    if (!shouldFire || functionName.empty()) return;

    lua_State* L = script.getState();
    pushNode(L, node);

    script.call(functionName.c_str(), 1, 0);
  }
};
}  // namespace gfx::ui