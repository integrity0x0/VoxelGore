#pragma once

#include "UiElement.h"

namespace gfx {
class UiText : public UiElement {
 public:

  [[nodiscard]] const std::wstring& GetText() const { return text_; }

  void SetText(std::wstring&& text) {
    text_ = std::move(text);
  }
 private:
  std::wstring text_;
};
}  // namespace gfx
