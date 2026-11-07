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

  const size_t rows = (children_.size() + columns_ - 1) / columns_;
  const glm::vec2 step = cell_ + gap_;

  size_t index = 0;
  for (size_t y = 0; y < rows; ++y) {
    for (size_t x = 0; x < columns_; ++x) {
      const UiLength2 pos{
          .x = {.value = inset_.x + x * step.x},
          .y = {.value = inset_.y + y * step.y},
      };

      const auto& child = children_[index];
      child->SetPos(pos);
      child->SetSize({.x = {.value = cell_.x}, .y = {.value = cell_.y}});

      if (++index >= children_.size()) return;
    }
  }
}

}  // namespace gfx