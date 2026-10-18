#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../../../script/LuaScript.h"
#include "UiElement.h"

namespace gfx {

enum class UiInputMode { 
  Passthrough, 
  Capture,
  Count
};

enum class UiPageType {
  Overlay,
  Screen,
  Count
};

struct UiPageProps {
  std::string id;
  UiInputMode input = UiInputMode::Passthrough;
  UiPageType type = UiPageType::Overlay;
};

class UiPage {
 public:
  UiPage(UiPageProps props, std::vector<std::unique_ptr<UiElement>>&& roots);

  [[nodiscard]] const UiPageProps& GetProps() const { return props_; }
  [[nodiscard]] const std::string& GetId() const { return props_.id; }
  [[nodiscard]] UiPageType GetType() const { return props_.type; }
  [[nodiscard]] UiInputMode GetInputMode() const { return props_.input; }

  void SetType(UiPageType type) {
    props_.type = type;
  }

  void SetInputMode(UiInputMode input) {
    props_.input = input;
  }

  [[nodiscard]] const std::vector<std::unique_ptr<UiElement>>& GetRoots() const { return roots_; }

  [[nodiscard]] UiElement* FindById(std::string_view id) const;

  std::vector<std::string> AttachScript(script::LuaScript&& script);
  [[nodiscard]] const std::optional<script::LuaScript>& GetScript() { return script_; }

  void UpdateLayout(const UiRect& screen);
  void Relayout(const UiRect& screen);
  void Render(UiRenderer& renderer);
 private:
  using ElementsById = std::unordered_map<std::string, UiElement*, util::StringHash, std::equal_to<>>;
  void IndexById(UiElement& element, ElementsById& out);

 private:
  UiPageProps props_;
  std::vector<std::unique_ptr<UiElement>> roots_;
  ElementsById byId_;
  std::optional<script::LuaScript> script_;
};

}  // namespace gfx