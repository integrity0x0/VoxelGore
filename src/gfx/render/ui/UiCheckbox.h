#pragma once

#include "UiContainer.h"

namespace gfx {
class UiCheckbox : public UiContainer {
 public:
  UiCheckbox(std::string_view id, const UiElement* parent) : UiContainer(id, parent) {}

  [[nodiscard]] UiElementType GetNodeType() const override { return UiElementType::Checkbox; }

  [[nodiscard]] bool IsCheckable() const override { return true; }
  [[nodiscard]] bool IsChecked() const override { return IsState(UiStateFlags::Checked); }

  void SetChecked(bool checked) override {
    if (IsChecked() == checked) return;
    SetStateFlag(UiStateFlags::Checked, checked);
    //if (checked) parent_->OnChildChecked(this);
  }

  void Toggle() { SetChecked(!IsChecked()); }
};
}  // namespace gfx