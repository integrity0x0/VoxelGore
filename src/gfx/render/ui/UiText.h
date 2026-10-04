#pragma once

#include "UiElement.h"

namespace gfx {
class UiText : public UiElement {
 public:
  UiText(std::string_view id, const UiElement* parent) : UiElement(id, parent) {}
  [[nodiscard]] const std::wstring& GetText() const { return text_; }

  void SetText(std::wstring&& text) {
    text_ = std::move(text);
  }

  
  ~UiText() override {}
 private:
  std::wstring text_;
};
}  // namespace gfx
