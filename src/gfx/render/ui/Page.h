#pragma once

#include <tinyxml2.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "../../../script/LuaScript.h"
#include "../../../vkcore/commands/CommandPool.h"
#include "../../../vkcore/devices/Device.h"
#include "NodeParser.h"
#include "Renderer.h"
#include "TriggerDispatcher.h"

namespace gfx::ui {
class Page {
 public:
  Page(const vkcore::Device& device, const vkcore::CommandPool& commandPool,
       const script::LuaState& luaState, std::string_view xmlPath);
  const std::vector<std::shared_ptr<Node>>& getNodes() const { return nodes_; }
  void Update(glm::vec2 screenSize);
  void Render(Renderer& renderer);

 private:
  std::string xmlPath_;
  std::optional<script::LuaScript> script_;
  TriggerDispatcher triggerDispatcher_;
  std::vector<std::shared_ptr<Node>> nodes_;
};
}  // namespace gfx::ui