#include "UiBar.h"

namespace gfx {
void UiBar::Render(UiRenderer& renderer) {
  RenderBg(renderer);
  RenderContent(renderer);
  RenderBorder(renderer);
  RenderFill(renderer);
}

void UiBar::RenderFill(UiRenderer& renderer) {
  auto [pos, size] = ScaleAround(resolvedLayout_.rect.pos, resolvedLayout_.rect.size,
                                 GetPivotPoint(), resolvedVisual_.scale);

  const float r = (max_ > 0.0f) ? std::clamp(value_ / max_, 0.0f, 1.0f) : 0.0f;

  static constexpr glm::vec2 kAxis[] = {
      {1.0f, 0.0f},  // Left
      {1.0f, 0.0f},  // Right
      {0.0f, 1.0f},  // Top
      {0.0f, 1.0f},  // Bottom
      {1.0f, 0.0f},  // CenterX
      {0.0f, 1.0f},  // CenterY
  };

  static constexpr glm::vec2 kOffset[] = {
      {0.0f, 0.0f},  // Left
      {1.0f, 0.0f},  // Right
      {0.0f, 0.0f},  // Top
      {0.0f, 1.0f},  // Bottom
      {0.5f, 0.0f},  // CenterX
      {0.0f, 0.5f},  // CenterY
  };

  const auto fillFrom = static_cast<size_t>(resolvedVisual_.fillFrom);
  assert(fillFrom < static_cast<size_t>(UiFillFrom::Count));

  const glm::vec2 fullSize = size;
  const float invR = 1.0f - r;

  pos += fullSize * kOffset[fillFrom] * invR;
  size = fullSize * (glm::vec2(1.0f) - kAxis[fillFrom] * invR);

  renderer.Submit(pos, size, resolvedVisual_.fillImage, resolvedVisual_.fillColor,
                  resolvedVisual_.fillRadius);
}
}  // namespace gfx