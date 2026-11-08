#include "UiGrid.h"

namespace gfx {
void UiGrid::ApplyStyle(const UiStyle& style) {
  UiContainer::ApplyStyle(style);
  const UiGridProps& g = style.gridProps;
  if (g.columns) columns_ = std::max<size_t>(*g.columns, 1);
  if (g.cell) cell_ = *g.cell;
  if (g.gap) gap_ = *g.gap;
  if (g.inset) inset_ = *g.inset;

  ArrangeChildren();
}

glm::vec2 UiGrid::ResolveSize(const UiRect& parent) const {
  size_t rows = (children_.size() + columns_ - 1) / columns_;

  return glm::vec2(cell_.x * columns_, cell_.y * rows) + inset_ * 2.0f +
         glm::vec2((columns_ - 1) * gap_.x, (rows - 1) * gap_.y);
}

void UiGrid::ArrangeChildren() {
  if (children_.empty() || columns_ == 0) return;

  const glm::vec2 step = cell_ + gap_;

  for (size_t i = 0; i < children_.size(); ++i) {
    UiElement& child = *children_[i];

    const glm::vec2 cellIndex(static_cast<float>(i % columns_), static_cast<float>(i / columns_));
    const glm::vec2 pos = inset_ + cellIndex * step + cell_ * 0.5f;

    child.SetAnchor({UiAlignX::Left, UiAlignY::Top});
    child.SetPivot({UiAlignX::Center, UiAlignY::Center});
    child.SetSize({.x = {.value = cell_.x}, .y = {.value = cell_.y}});
    child.SetPos({.x = {.value = pos.x}, .y = {.value = pos.y}});
  }
}

}  // namespace gfx