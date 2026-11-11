#include "UiContainer.h"

namespace gfx {
void UiContainer::Render(UiRenderer& renderer) {
  UiElement::Render(renderer);

  for (auto& child : children_) {
    child->Render(renderer);
  }
}

void UiContainer::Relayout(const UiRect& rect) {
  UiElement::Relayout(rect);
  for (auto& child : children_) {
    child->Relayout(GetResolvedLayout().rect);
  }
}

void UiContainer::ApplyStyle(const UiStyle& s) {
  UiElement::ApplyStyle(s);
  if (s.selectionProps.min)
    selection_.SetMin(*s.selectionProps.min);
  if (s.selectionProps.max) selection_.SetMax(*s.selectionProps.max);
}

}  // namespace gfx