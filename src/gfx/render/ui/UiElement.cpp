#include "UiElement.h"

namespace gfx {

UiElement::UiElement(std::string_view id, const UiElement* parent) : id_(id), parent_(parent) {}

void UiElement::Render(UiRenderer& renderer) {
  RenderBg(renderer);

  renderer.Submit({pos_.x.value, pos_.y.value}, {size_.x.value, size_.y.value}, image_, color_,
                  radius_);
}

void UiElement::RenderBg(UiRenderer& renderer) {
  if (!padding_) return;

  const auto& padding = *padding_;

  const glm::vec2 pos{pos_.x.value - padding.left.value, pos_.y.value - padding.top.value};

  const glm::vec2 size{size_.x.value + padding.left.value + padding.right.value,
                       size_.y.value + padding.top.value + padding.bottom.value};

  renderer.Submit(pos, size, bgImage_, bgColor_, radius_);
}

void UiElement::Relayout(const UiRect& rect) {
  layoutSize_.x =
      size_.x.unit == UiUnit::Percent ? size_.x.value * rect.size.x / 100.0f : size_.x.value;

  layoutSize_.y =
      size_.y.unit == UiUnit::Percent ? size_.y.value * rect.size.y / 100.0f : size_.y.value;

  const float posX =
      pos_.x.unit == UiUnit::Percent ? pos_.x.value * rect.size.x / 100.0f : pos_.x.value;

  const float posY =
      pos_.y.unit == UiUnit::Percent ? pos_.y.value * rect.size.y / 100.0f : pos_.y.value;

  const float anchorFactorX = kAlignmentFactors[static_cast<size_t>(anchor_.x)];
  const float anchorFactorY = kAlignmentFactors[static_cast<size_t>(anchor_.y)];

  const float pivotFactorX = kAlignmentFactors[static_cast<size_t>(pivot_.x)];
  const float pivotFactorY = kAlignmentFactors[static_cast<size_t>(pivot_.y)];

  layoutPos_.x = rect.pos.x + rect.size.x * anchorFactorX + posX - layoutSize_.x * pivotFactorX;

  layoutPos_.y = rect.pos.y + rect.size.y * anchorFactorY + posY - layoutSize_.y * pivotFactorY;

  const UiRect childRect{.pos = layoutPos_, .size = layoutSize_};

  for (auto& child : children_) child->Relayout(childRect);
}
}  // namespace gfx