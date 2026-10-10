#pragma once

#include <glm/glm.hpp>

#include "UiElement.h"
#include "../../../core/InputState.h"

namespace gfx {

struct UiCameraSwipe {
  glm::vec2 deltaPx;
};

class UiInputRouter {
 public:
  UiCameraSwipe RouteInput(const core::InputState& input,
                           const std::vector<std::unique_ptr<UiElement>>& roots);
  void UpdateHover(UiElement* newHover);

  void ProcessPointer(int32_t id, const core::Pointer& p,
                      std::span<const std::unique_ptr<UiElement>> roots, UiCameraSwipe& swipe);
  void Reset();

 private:
  struct TouchOwner {
    UiElement* element = nullptr;
    bool camera = false;
  };

  [[nodiscard]] UiElement* HitTest(glm::vec2 pos,
                                   std::span<const std::unique_ptr<UiElement>> element);

 private:
  [[nodiscard]] bool IsValidOwner(size_t pointerId) const;
  void ProcessPointer(const core::Pointer& pointer);
 private:
  std::vector<TouchOwner> owners_; 
  UiElement* hovered_ = nullptr;
};
}  // namespace gfx
