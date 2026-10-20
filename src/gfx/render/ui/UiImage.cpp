#include "UiImage.h"

namespace gfx {
void UiImage::Render(UiRenderer& renderer) {
  RenderBg(renderer);

  const glm::vec2& pos = resolvedLayout_.rect.pos;
  const glm::vec2& size = resolvedLayout_.rect.size;

  renderer.Submit(pos, size, src_, color_, radius_);

  RenderBorder(renderer);
}
}  // namespace gfx