#pragma once

#include <memory>

#include "UiElement.h"

namespace gfx {

enum class UiStretch { Disabled, Horizontal, Vertical };

class UiContainer final : public UiElement {
 public:
  [[nodiscard]] UiStretch GetStretch() const { return stretch_; }
  void SetStretch(UiStretch stretch) { stretch_ = stretch; }

 private:
  UiStretch stretch_;
};
}  // namespace gfx
