#include "UiInputRouter.h"

namespace gfx {
UiCameraSwipe UiInputRouter::RouteInput(const core::InputState& input,
                                        std::span<UiPage* const> pages, bool captured,
                                        UiEventQueue& queue) {
  UiCameraSwipe cameraSwipe = {};
  for (const auto& pointer : input.GetPointers()) {
    ProcessPointer(pointer.first, pointer.second, pages, captured, cameraSwipe, queue);
  }

  if (input.HasCursor() && !input.IsCursorDisabled()) {
    UpdateHover(HitTestPages({input.GetCursorX(), input.GetCursorY()}, pages), queue);
  } else {
    UpdateHover(nullptr, queue);
  }

  return cameraSwipe;
}

void UiInputRouter::UpdateHover(UiElement* newHovered, UiEventQueue& queue) {
  if (newHovered == hovered_) return;
  if (hovered_) {
    hovered_->RemoveStateFlags(UiStateFlags::Hovered);
    queue.emplace(UiEventType::Unhovered, *hovered_);
  }
  if (newHovered) {
    newHovered->AddStateFlags(UiStateFlags::Hovered);
    queue.emplace(UiEventType::Hovered, *newHovered);
  }
  hovered_ = newHovered;
}

void UiInputRouter::CancelAll(UiEventQueue& queue) {
  for (TouchOwner& owner : owners_) ReleaseOwner(owner, false, queue);
}

void UiInputRouter::Reset() {
  owners_.fill({});
  hovered_ = nullptr;
}

void UiInputRouter::ProcessPointer(int32_t id, const core::Pointer& pointer,
                                   std::span<UiPage* const> pages, bool captured,
                                   UiCameraSwipe& swipe, UiEventQueue& queue) {
  if (id < 0 || static_cast<size_t>(id) >= owners_.size()) return;
  using Phase = core::Pointer::Phase;

  switch (pointer.phase) {
    case Phase::Began: {
      BeginTouch(owners_[id], pointer, pages, captured, queue);
      break;
    }
    case Phase::Moved: {
      MoveTouch(owners_[id], pointer, swipe);
      break;
    }
    case Phase::Cancelled:
    case Phase::Ended:
    case Phase::None: {
      EndTouch(owners_[id], pointer, queue);
      break;
    }
  }
}

void UiInputRouter::BeginTouch(TouchOwner& owner, const core::Pointer& p,
                               std::span<UiPage* const> pages, bool captured, UiEventQueue& queue) {
  using Role = TouchOwner::Role;

  ReleaseOwner(owner, false, queue);

  UiElement* hit = HitTestPages({p.currentX, p.currentY}, pages);
  if (!hit) {
    if (!captured) owner.role = Role::Camera;
    return;
  }
  if (hit->IsState(UiStateFlags::Held)) {
    owner.role = Role::Blocked;
    return;
  }

  owner = {Role::Element, hit};
  hit->AddStateFlags(UiStateFlags::Held);
  queue.emplace(UiEventType::Clicked, *hit);
}

void UiInputRouter::MoveTouch(TouchOwner& owner, const core::Pointer& p,
                              UiCameraSwipe& cameraSwipe) {
  if (owner.role == TouchOwner::Role::Camera) {
    cameraSwipe.deltaPx = {p.deltaX, p.deltaY};
  }
}

void UiInputRouter::EndTouch(TouchOwner& owner, const core::Pointer& p, UiEventQueue& queue) {
  bool released = owner.role == TouchOwner::Role::Element &&
                  p.phase == core::Pointer::Phase::Ended &&
                  owner.element->Contains({p.currentX, p.currentY});
  ReleaseOwner(owner, released, queue);
}

void UiInputRouter::ReleaseOwner(TouchOwner& owner, bool released, UiEventQueue& queue) {
  if (owner.role == TouchOwner::Role::Element) {
    UiElement& element = *owner.element;
    element.RemoveStateFlags(UiStateFlags::Held);
    queue.emplace(released ? UiEventType::Released : UiEventType::Cancelled, element);
  }
  owner = {};
}

UiElement* UiInputRouter::HitTestPages(glm::vec2 pos, std::span<UiPage* const> pages) const {
  for (size_t i = pages.size(); i-- > 0;) {
    if (UiElement* hit = HitTest(pos, pages[i]->GetRoots())) return hit;
  }
  return nullptr;
}

UiElement* UiInputRouter::HitTest(glm::vec2 pos,
                                  std::span<const std::unique_ptr<UiElement>> elements) const {
  for (size_t i = elements.size(); i-- > 0;) {
    UiElement& el = *elements[i];
    if (UiElement* child = HitTest(pos, el.GetChildren())) return child;
    if (el.Contains(pos) && el.IsEffectivelyInteractive()) return &el; 
  }
  return nullptr;
}
}  // namespace gfx