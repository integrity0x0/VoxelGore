#include "UiElement.h"

namespace gfx {

UiElement::UiElement(std::string_view id, const UiElement* parent) : id_(id), parent_(parent) {}

void UiElement::Render(UiRenderer& renderer) {
  RenderBg(renderer);
  renderer.Submit(resolvedLayout_.rect.pos, resolvedLayout_.rect.size, image_, color_,
                  radius_);
}

void UiElement::RenderBg(UiRenderer& renderer) {
  if (!padding_) return;

  const auto& p = resolvedLayout_.padding;
  const auto& r = resolvedLayout_.rect;

  const glm::vec2 pos(r.pos.x - p.left, r.pos.y - p.top);
  const glm::vec2 size(r.size.x + p.left + p.right, r.size.y + p.top + p.bottom);

  renderer.Submit(pos, size, bgImage_, bgColor_, radius_);
}

void UiElement::Relayout(const UiRect& rect) {
  auto& layoutSize = resolvedLayout_.rect.size;
  layoutSize.x =
      size_.x.unit == UiUnit::Percent ? size_.x.value * rect.size.x / 100.0f : size_.x.value;

  layoutSize.y =
      size_.y.unit == UiUnit::Percent ? size_.y.value * rect.size.y / 100.0f : size_.y.value;

  const float posX =
      pos_.x.unit == UiUnit::Percent ? pos_.x.value * rect.size.x / 100.0f : pos_.x.value;

  const float posY =
      pos_.y.unit == UiUnit::Percent ? pos_.y.value * rect.size.y / 100.0f : pos_.y.value;

  const float anchorFactorX = kAlignmentFactors[static_cast<size_t>(anchor_.x)];
  const float anchorFactorY = kAlignmentFactors[static_cast<size_t>(anchor_.y)];

  const float pivotFactorX = kAlignmentFactors[static_cast<size_t>(pivot_.x)];
  const float pivotFactorY = kAlignmentFactors[static_cast<size_t>(pivot_.y)];

  auto& layoutPos = resolvedLayout_.rect.pos;

  layoutPos.x = rect.pos.x + rect.size.x * anchorFactorX + posX - layoutSize.x * pivotFactorX;

  layoutPos.y = rect.pos.y + rect.size.y * anchorFactorY + posY - layoutSize.y * pivotFactorY;
  
  if (padding_) {
    auto& padding = resolvedLayout_.padding;

    padding.left = padding_->left.unit == UiUnit::Percent
                       ? padding_->left.value / 100.0f * rect.size.x
                       : padding_->left.value;

    padding.top = padding_->top.unit == UiUnit::Percent
                       ? padding_->top.value / 100.0f * rect.size.y
                       : padding_->top.value;

    padding.right = padding_->right.unit == UiUnit::Percent
                       ? padding_->right.value / 100.0f * rect.size.x
                       : padding_->right.value;

    padding.bottom = padding_->bottom.unit == UiUnit::Percent
                       ? padding_->bottom.value / 100.0f * rect.size.y
                       : padding_->bottom.value;
  }
}
}  // namespace gfx