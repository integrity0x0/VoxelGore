#include "UiText.h"

namespace gfx {
namespace {

struct GlyphQuad {
  glm::vec2 pos;
  glm::vec2 size;
  GlyphSlot slot;  // ← было UiTextureRegion
};

struct TextLayout {
  std::vector<GlyphQuad> quads;
  float width = 0.0f;
  float height = 0.0f;
  float baseline = 0.0f;
};

TextLayout LayoutText(const std::u32string& text, Font& font, float pixelSize) {
  TextLayout layout;
  if (text.empty()) return layout;

  const FontMetrics& metrics = font.GetMetrics(static_cast<uint32_t>(pixelSize));

  layout.height = metrics.lineHeight;
  layout.baseline = metrics.ascent;

  float penX = 0.0f;
  float minX = 0.0f;
  float maxX = 0.0f;

  for (char32_t c : text) {
    const Glyph& glyph = font.GetGlyph(c, static_cast<uint32_t>(pixelSize));

    const float x = penX + static_cast<float>(glyph.metrics.bearing.x);
    const float y = layout.baseline - static_cast<float>(glyph.metrics.bearing.y);

    const float glyphWidth = static_cast<float>(glyph.metrics.size.x);

    if (glyph.slot.materialId != kInvalidMaterialId) {
      layout.quads.push_back(
          {.pos = {x, y}, .size = glm::vec2(glyph.metrics.size), .slot = glyph.slot});

      minX = std::min(minX, x);
      maxX = std::max(maxX, x + glyphWidth);
    }

    penX += glyph.metrics.advance;
  }

  layout.width = std::max(penX, maxX) - minX;

  for (GlyphQuad& quad : layout.quads) {
    quad.pos.x -= minX;
  }

  return layout;
}

}  // namespace

void UiText::UpdateVisual() const {
  UiElement::UpdateVisual();
  // text_ уже source of truth, в resolvedVisual_.text класть не обязательно
}

void UiText::Render(UiRenderer& renderer) {
  if (!IsEffectivelyVisible()) return;

  RenderBg(renderer);
  RenderText(renderer);
  // RenderBorder(renderer); // если нужен бордер у текстового элемента
}

void UiText::RenderText(UiRenderer& renderer) {
  const UiResolvedVisual& v = GetResolvedVisual();
  if (v.text.empty() || !v.font) return;

  FontStyle style{.weight = v.fontWeight, .italic = v.italic};

  const Font* font = v.font->FindFont(style);
  if (!font) return;

  const float pixelSize = v.fontSize * v.scale;
  if (pixelSize <= 0.0f) return;

  TextLayout layout = LayoutText(v.text, *const_cast<Font*>(font), pixelSize);
  if (layout.quads.empty()) return;

  const UiRect& content = resolvedLayout_.rect;
  const glm::vec2 pivot = GetPivotPoint();

  const float offsetX = 0.0f;
  const float offsetY = 0.0f;

  const glm::vec2 textOrigin = content.pos + glm::vec2(offsetX, offsetY);

  const bool hasShadow = v.shadowColor.a > 0.001f;
  const glm::vec2 shadowOffset = {1.0f, 1.0f};

  auto drawQuads = [&](const glm::vec4& color, const glm::vec2& extraOffset) {
    for (const GlyphQuad& q : layout.quads) {
      const glm::vec2 glyphPos = textOrigin + q.pos + extraOffset;

      auto [pos, size] = ScaleAround(glyphPos, q.size, pivot, v.scale);

      renderer.SubmitGlyph(pos, size, q.slot, color);
    }
  };

  if (hasShadow) {
    drawQuads(v.shadowColor, shadowOffset * v.scale);
  }

  drawQuads(v.color, {0.0f, 0.0f});
}

}  // namespace gfx