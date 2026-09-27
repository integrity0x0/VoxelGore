#pragma once

#include "UiElement.h"

namespace gfx {
class UiText : public UiElement {
 public:

  [[nodiscard]] const std::wstring& text() const { return text_; }

  void setText(std::wstring&& text) {
    text_ = std::move(text);
  }
 private:
  std::wstring text_;
};
}  // namespace gfx
