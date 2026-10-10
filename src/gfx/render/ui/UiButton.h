#pragma once

#include "UiContainer.h"

namespace gfx {
class UiButton final : public UiContainer {
 public:
  UiButton(std::string_view id, const UiElement* parent)
	  : UiContainer(id, parent) {}
};
}  // namespace gfx
