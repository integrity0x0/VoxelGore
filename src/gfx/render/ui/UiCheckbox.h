#pragma once

#include "UiContainer.h"

namespace gfx {
class UiCheckbox final : public UiContainer {
 public:
  UiCheckbox(std::string_view id, const UiElement* parent) : UiContainer(id, parent) {}

  [[nodiscard]] UiElementType GetNodeType() const override { return UiElementType::Checkbox; }

  [[nodiscard]] bool IsChecked() const { return checked_; }

  void SetChecked(bool checked) {
    checked_ = checked;
  }

  void Toggle() {
    checked_ = !checked_;
  }
 private:
  bool checked_ = false;
};
}  // namespace gfx
