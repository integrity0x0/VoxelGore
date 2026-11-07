#include "UiText.h"

#include <algorithm>
#include <cmath>

namespace gfx {

void UiText::UpdateVisual() const {
  UiElement::UpdateVisual();
  UpdateTextLayout();
}

void UiText::UpdateTextLayout() const {
   const UiResolvedVisual& v = resolvedVisual_;
  UiTextLayout& out = resolvedTextLayout_;

  Font* font = v.font ? v.font->FindFont({v.fontWeight, v.italic}) : nullptr;
  const uint32_t pixelSize = static_cast<uint32_t>(std::max(1.0f, std::round(v.fontSize)));

  if (out.font == font && out.pixelSize == pixelSize && out.text == v.text) return;

  out.font = font;
  out.pixelSize = pixelSize;
  out.text = v.text;
  out.quads.clear();
  out.size = glm::vec2(0.0f);

  if (!font || v.text.empty() || v.fontSize <= 0.0f) return;

  const FontMetrics& metrics = font->GetMetrics(pixelSize);
  const float ascent = metrics.ascent;
  const float lineHeight = metrics.lineHeight;

  out.quads.reserve(v.text.size());

  float penX = 0.0f;
  float lineRight = 0.0f;
  float maxWidth = 0.0f;
  size_t line = 0;

  for (char32_t c : v.text) {
    if (c == U'\n') {
      maxWidth = std::max(maxWidth, lineRight);
      ++line;
      penX = 0.0f;
      lineRight = 0.0f;
      continue;
    }

    const Glyph& glyph = font->GetGlyph(c, pixelSize);
    const GlyphMetrics& gm = glyph.metrics;

    if (glyph.slot.materialId != kInvalidMaterialId) {
      const glm::vec2 size(gm.size);
      const glm::vec2 pos(
          penX + static_cast<float>(gm.bearing.x),
          static_cast<float>(line) * lineHeight + ascent - static_cast<float>(gm.bearing.y));

      out.quads.push_back({.pos = pos, .size = size, .slot = glyph.slot});
      lineRight = std::max(lineRight, pos.x + size.x);
    }

    penX += gm.advance;
    lineRight = std::max(lineRight, penX);
  }

  maxWidth = std::max(maxWidth, lineRight);
  out.size = {maxWidth, static_cast<float>(line + 1) * lineHeight};
}

glm::vec2 UiText::ResolveSize(const UiRect& parent) const {
  const bool autoX = size_.x.unit == UiUnit::Auto;
  const bool autoY = size_.y.unit == UiUnit::Auto;

  glm::vec2 result(autoX ? 0.0f : ResolveLength(size_.x, parent.size.x),
                   autoY ? 0.0f : ResolveLength(size_.y, parent.size.y));

  if (autoX || autoY) {
    std::ignore = GetResolvedVisual();

    const glm::vec2& measured = resolvedTextLayout_.size;
    if (autoX) result.x = measured.x;
    if (autoY) result.y = measured.y;
  }
  return result;
}

void UiText::Render(UiRenderer& renderer) {
  if (!IsEffectivelyVisible()) return;

  RenderBg(renderer);
  RenderText(renderer);
}

void UiText::RenderText(UiRenderer& renderer) {
  const UiResolvedVisual& v = GetResolvedVisual();
  const std::vector<UiGlyphQuad>& quads = resolvedTextLayout_.quads;
  if (quads.empty()) return;

  const glm::vec2 origin = resolvedLayout_.rect.pos;
  const glm::vec2 pivot = GetPivotPoint();

  auto draw = [&](const glm::vec4& color, const glm::vec2& extraOffset) {
    for (const UiGlyphQuad& q : quads) {
      auto [pos, size] = ScaleAround(origin + q.pos + extraOffset, q.size, pivot, v.scale);
      renderer.SubmitGlyph(pos, size, q.slot, color);
    }
  };

  if (v.textShadow && v.shadowColor.a > 0.001f) draw(v.shadowColor, v.shadowOffset);

  draw(v.color, {0.0f, 0.0f});
}

}  // namespace gfx