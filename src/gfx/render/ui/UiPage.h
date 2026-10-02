#pragma once

#include <string>
#include <string_view>
#include <optional>

#include "../../../script/LuaScript.h"
#include "UiElement.h"

namespace gfx {
class UiPage {
 public:
  UiPage(std::string_view id, std::vector<std::unique_ptr<UiElement>>&& elements);
 private:
  
  std::optional<script::LuaScript> luaScript_;

};
}  // namespace gfx
