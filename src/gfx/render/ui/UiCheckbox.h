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
    if (checked) const_cast<UiElement*>(parent_)->OnChildCheckedChanged(*this);
  }

  void Toggle() { 
      SetChecked(!IsChecked()); 
  }

  void OnReleased() override {
    if (auto* group = dynamic_cast<UiContainer*>(const_cast<UiElement*>(parent_)))
      group->GetSelection().OnItemClicked(this);
    else
      Toggle();
  }
};
}  // namespace gfx