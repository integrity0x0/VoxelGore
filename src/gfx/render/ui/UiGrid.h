#pragma once

#include "UiContainer.h"

namespace gfx {
class UiGrid : public UiContainer {
 public:
  UiGrid(std::string_view id, const UiElement* parent) : UiContainer(id, parent) {}

 private:
};
}  // namespace gfx
