#pragma once

#include "UiElement.h"

namespace gfx {
class UiText final : public UiElement {
 public:
  UiText(std::string_view id, const UiElement* parent) : UiElement(id, parent) {}

  [[nodiscard]] UiElementType GetNodeType() const override { return UiElementType::Text; }

  [[nodiscard]] const std::u8string& GetText() const { return text_; }

  void SetText(std::u8string&& text) {
    text_ = std::move(text);
  }

  ~UiText() override {}
 private:
  std::u8string text_;
};
}  // namespace gfx