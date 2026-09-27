#pragma once

#include <memory>

#include "UiElement.h"

namespace gfx {

enum class UiStretch { Disabled, Horizontal, Vertical };

class UiContainer final : public UiElement {
 public:
  
 private:
  UiStretch stretch_;
};
}  // namespace gfx
