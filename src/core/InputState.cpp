#include "InputState.h"

namespace core {

InputState::InputState() : keyStates_({}), mouseButtonStates_({}) {}

void InputState::setKeyState(uint32_t keyCode, ButtonState state) {
  if (keyCode <= KEY_LAST) {
    keyStates_[keyCode] = state;
  }
}

void InputState::setMouseButtonState(uint32_t buttonIndex, ButtonState state) {
  if (buttonIndex < MOUSE_BUTTON_LAST) {
    mouseButtonStates_[buttonIndex] = state;
  }
}

void InputState::Reset() {
  cursorDeltaX_ = cursorDeltaY_ = 0.0f;

  for (auto& state : keyStates_) {
    switch (state) {
      case ButtonState::Pressed:
        state = ButtonState::Held;
        break;
      case ButtonState::Released:
        state = ButtonState::Up;
        break;
      default:
        break;
    }
  }

  for (auto& state : mouseButtonStates_) {
    switch (state) {
      case ButtonState::Pressed:
        state = ButtonState::Held;
        break;
      case ButtonState::Released:
        state = ButtonState::Up;
        break;
      default:
        break;
    }
  }

  for (auto it = pointers_.begin(); it != pointers_.end();) {
    it->second.deltaX = 0.0f;
    it->second.deltaY = 0.0f;
    it->second.prevX = it->second.currentX;
    it->second.prevY = it->second.currentY;
    if (it->second.phase == Pointer::Phase::Ended ||
        it->second.phase == Pointer::Phase::Cancelled) {
      it = pointers_.erase(it);
    } else {
      it++;
    }
  }
}

}  // namespace core