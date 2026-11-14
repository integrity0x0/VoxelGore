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
    if (auto* group = dynamic_cast<UiContainer*>(const_cast<UiElement*>(parent_)))
      group->GetSelection().OnItemClicked(this);
    else
      SetCheckedRaw(checked);  
  }

  void Toggle() { 
    SetChecked(!IsChecked()); 
  }

  void OnReleased() override {
    Toggle();
  }

  void SetCheckedRaw(bool checked) { SetStateFlag(UiStateFlags::Checked, checked); }
};
}  // namespace gfx