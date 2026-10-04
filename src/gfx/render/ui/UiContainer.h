#pragma once

#include <memory>

#include "UiElement.h"

namespace gfx {

enum class UiStretch { Disabled, Horizontal, Vertical };

class UiContainer final : public UiElement {
 public:
  UiContainer(std::string_view id, const UiElement* parent) : UiElement(id, parent) {}
  [[nodiscard]] UiStretch GetStretch() const { return stretch_; }
  void SetStretch(UiStretch stretch) { stretch_ = stretch; }

  ~UiContainer() override {}
 private:
  UiStretch stretch_;
};
}  // namespace gfx
