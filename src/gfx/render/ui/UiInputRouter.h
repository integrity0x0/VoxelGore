#pragma once

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <span>

#include "../../../core/InputState.h"
#include "UiElement.h"
#include "UiEvents.h"
#include "UiPage.h"

namespace gfx {

struct UiCameraSwipe {
  glm::vec2 deltaPx = glm::vec2(0.0f);
};

class UiInputRouter {
 public:
  UiCameraSwipe RouteInput(const core::InputState& input, std::span<UiPage* const> pages,
                           bool captured, UiEventQueue& queue);

  void UpdateHover(UiElement* newHover, UiEventQueue& queue);

  void CancelAll(UiEventQueue& queue);

  void Reset();

 private:
  struct TouchOwner {
    enum class Role : uint8_t { None, Element, Blocked, Camera };

    Role role = Role::None;
    UiElement* element = nullptr;
  };

  void ProcessPointer(int32_t id, const core::Pointer& p, std::span<UiPage* const> pages,
                      bool captured, UiCameraSwipe& swipe, UiEventQueue& queue);

  void BeginTouch(TouchOwner& owner, const core::Pointer& p, std::span<UiPage* const> pages,
                  bool captured, UiEventQueue& queue);

  void MoveTouch(TouchOwner& owner, const core::Pointer& p, UiCameraSwipe& cameraSwipe);

  void EndTouch(TouchOwner& owner, const core::Pointer& p, UiEventQueue& queue);

  void ReleaseOwner(TouchOwner& owner, bool released, UiEventQueue& queue);

  [[nodiscard]] UiElement* HitTestPages(glm::vec2 pos, std::span<UiPage* const> pages) const;

  [[nodiscard]] UiElement* HitTest(glm::vec2 pos,
                                   std::span<const std::unique_ptr<UiElement>> elements) const;

 private:
  std::array<TouchOwner, core::InputState::kMaxPointers> owners_;
  UiElement* hovered_ = nullptr;
};
}  // namespace gfx