#pragma once

#include <functional>
#include <queue>

#include "UiElement.h"
#include "UiTypes.h"

namespace gfx {
struct UiEvent {
  UiEventType type;
  std::reference_wrapper<UiElement> element;

  UiEvent(UiEventType type, UiElement& element) 
	  : type(type), element(element) {}
};

using UiEventQueue = std::queue<UiEvent>;

}  // namespace gfx