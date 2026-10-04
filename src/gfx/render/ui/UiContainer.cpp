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
}  // namespace gfx