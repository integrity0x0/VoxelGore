#pragma once
#include "UiElement.h"

namespace gfx {
class UiStick : public UiElement {
 public:
 private:
  glm::vec2 value_ = glm::vec2(0.0f);
};
}  // namespace gfx