#include "UiInputRouter.h"

namespace gfx {
UiCameraSwipe UiInputRouter::RouteInput(const core::InputState& input,
                                          const std::vector<std::unique_ptr<UiElement>>& roots) {
  return {};
  for (const auto& pointer : input.GetPointers()) {
    
  }
}

bool UiInputRouter::IsValidOwner(size_t pointerId) const {
  return pointerId < owners_.size() && owners_[pointerId].element;
}

UiElement* UiInputRouter::HitTest(glm::vec2 pos,
                                  std::span<const std::unique_ptr<UiElement>> elements) {
  UiElement* hit = nullptr;
  for (auto& el : elements) {
    if (UiElement* childHit = HitTest(pos, el->GetChildren())) {
      hit = childHit;
    } else if (el->Contains(pos)) {
      hit = el.get();
    }
  }
  return hit;
}

}  // namespace gfx