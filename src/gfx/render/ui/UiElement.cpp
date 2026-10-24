#include "UiElement.h"

namespace gfx {

UiRect UiElement::ScaleAround(glm::vec2 pos, glm::vec2 size, glm::vec2 origin, float scale) {
  return {origin + (pos - origin) * scale, size * scale};
}

glm::vec2 UiElement::GetPivotPoint() const {
  const UiRect& r = resolvedLayout_.rect;
  return {r.pos.x + r.size.x * kAlignmentFactors[static_cast<size_t>(pivot_.x)],
          r.pos.y + r.size.y * kAlignmentFactors[static_cast<size_t>(pivot_.y)]};
}

void UiElement::Render(UiRenderer& renderer) {
  if (IsEffectivelyVisible()) {
    RenderBg(renderer);   
    RenderContent(renderer);
    RenderBorder(renderer);
  }
}

void UiElement::RenderBg(UiRenderer& renderer) {
  if (!padding_) return;

  const UiResolvedVisual& v = GetResolvedVisual();
  const auto& p = resolvedLayout_.padding;
  const auto& r = resolvedLayout_.rect;

  const glm::vec2 bgPos(r.pos.x - p.left, r.pos.y - p.top);
  const glm::vec2 bgSize(r.size.x + p.left + p.right, r.size.y + p.top + p.bottom);

  const auto [pos, size] = ScaleAround(bgPos, bgSize, GetPivotPoint(), v.scale);
  renderer.Submit(pos, size, v.bgImage, v.bgColor, v.bgRadius * v.scale);
}

void UiElement::RenderContent(UiRenderer& renderer) {
  const UiResolvedVisual& v = GetResolvedVisual();
  const UiRect& r = resolvedLayout_.rect;
  const auto [pos, size] = ScaleAround(r.pos, r.size, GetPivotPoint(), v.scale);
  renderer.Submit(pos, size, v.src, v.color, v.radius * v.scale);
}

void UiElement::RenderBorder(UiRenderer& renderer) {
  const UiResolvedVisual& v = GetResolvedVisual();
  if (v.borderWidth <= 0.0f) return;

  const UiRect& r = resolvedLayout_.rect;

  const auto [pos, size] = ScaleAround(r.pos, r.size, GetPivotPoint(), v.scale);
  renderer.Submit(pos, size, v.borderImage, v.borderColor, v.radius * v.scale,
                  v.borderWidth * v.scale);
}

void UiElement::ApplyStyle(const UiStyle& s) {
  MergeBaseVisual(s.visualProps);

  const UiLayoutProps& l = s.layoutProps;
  if (l.pos) SetPos(*l.pos);
  if (l.size) SetSize(*l.size);
  if (l.anchor) SetAnchor(*l.anchor);
  if (l.pivot) SetPivot(*l.pivot);
  if (l.padding) SetPadding(*l.padding);

  for (size_t i = 0; i < s.stateOverrides.size(); ++i) {
    GetStateOverride(static_cast<UiState>(i)).MergeFrom(s.stateOverrides[i]);
  }
}

void UiElement::UpdateVisual() const {
  visualDirty_ = false;

  UiVisualProps merged = baseVisual_;
  if (parent_) merged.scale = parent_->GetResolvedVisual().scale;

  for (uint32_t i = 0; i < static_cast<uint32_t>(UiState::Count); ++i) {
    const auto flag = static_cast<UiStateFlags>(1u << i);
    if (!IsState(flag)) continue;

    merged.MergeFrom(stateOverrides_[i]);
  }

  UiResolvedVisual r;
  merged.ApplyTo(r);
  if (!merged.bgRadius) r.bgRadius = r.radius;
  if (!merged.fillRadius) r.fillRadius = r.radius;
  resolvedVisual_ = std::move(r);
}

void UiElement::UpdateLayout(const UiRect& parentRect) {
  if (layoutDirty_) {
    Relayout(parentRect);
    return;
  }
  if (!childDirty_) return;

  childDirty_ = false;
  const UiRect content = resolvedLayout_.rect;
  for (const auto& child : GetChildren()) child->UpdateLayout(content);
}

void UiElement::Relayout(const UiRect& rect) {
  layoutDirty_ = false;
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

    padding.top =
        padding_->top.unit == UiUnit::Percent ? padding_->top.value / 100.0f * rect.size.y
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