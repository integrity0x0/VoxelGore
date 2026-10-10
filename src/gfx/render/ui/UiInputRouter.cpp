#include "UiInputRouter.h"

namespace gfx {
UiCameraSwipe UiInputRouter::RouteInput(const core::InputState& input,
                                        const std::vector<std::unique_ptr<UiElement>>& roots) {
  UiCameraSwipe cameraSwipe = {};
  for (const auto& pointer : input.GetPointers()) {
    ProcessPointer(pointer.first, pointer.second, roots, cameraSwipe);
  }

  if (input.HasCursor()) {
    UpdateHover(HitTest(glm::vec2(0.0f), roots));
  }

  return {};
}

void UiInputRouter::UpdateHover(UiElement* newHovered) {
  if (newHovered == hovered_) return;
  if (hovered_) {
    hovered_->ApplyHover(false);
  }
  if (newHovered) {
    newHovered->ApplyHover(true);
  }
  hovered_ = newHovered;
}

void UiInputRouter::Reset() {
  owners_.clear();
  hovered_ = nullptr;
}

void UiInputRouter::ProcessPointer(int32_t id, const core::Pointer& pointer,
                                   std::span<const std::unique_ptr<UiElement>> roots,
                                   UiCameraSwipe& swipe) {
  using Phase = core::Pointer::Phase;
  switch (pointer.phase) { 
    case Phase::Began : {
      if (UiElement* element = HitTest({pointer.currentX, pointer.currentY}, roots)) {
        if (!element->IsHeld()) element->ApplyHeld(true);
      }
      break;
    }
    case Phase::Cancelled:
    case Phase::Ended:
    case Phase::None: {
      if (id > owners_.size()) return;

      break;
    }
  }
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