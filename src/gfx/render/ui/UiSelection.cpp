#include "UiSelection.h"

#include <algorithm>

namespace gfx {

void UiSelection::Select(UiElement* item) {
  if (limits_.max == 0) return;
  if (!item->IsCheckable() || IsSelected(item)) return;
  item->SetChecked(true);
  selected_.push_back(item);
  TrimToMax();
}

void UiSelection::Deselect(UiElement* item) {
  if (selected_.size() <= limits_.min) return;
  auto it = std::find(selected_.begin(), selected_.end(), item);
  if (it == selected_.end()) return;
  item->SetChecked(false);
  selected_.erase(it);
}

void UiSelection::Clear() {
  while (selected_.size() > limits_.min) {
    selected_.front()->SetChecked(false);
    selected_.erase(selected_.begin());
  }
}

void UiSelection::OnItemClicked(UiElement* item) {
  if (item->IsChecked())
    Deselect(item);
  else
    Select(item);
}

void UiSelection::TrimToMax() {
  while (selected_.size() > limits_.max) {
    selected_.front()->SetChecked(false);
    selected_.erase(selected_.begin());
  }
}

void UiSelection::FillToMin(std::span<const std::unique_ptr<UiElement>> candidates) {
  for (const auto& c : candidates) {
    if (selected_.size() >= limits_.min) break;
    if (c->IsCheckable() && !IsSelected(c.get())) Select(c.get());
  }
} 

}  // namespace gfx