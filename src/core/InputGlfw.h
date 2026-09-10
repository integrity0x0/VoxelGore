#pragma once

#include <GLFW/glfw3.h>

#include "InputState.h"

namespace core {

class InputGlfw {
 public:
  [[nodiscard]] const InputState& getState() const { return state_; }
  [[nodiscard]] InputState& getState() { return state_; }

  void endFrame() { state_.Reset(); }

  void onKey(int key, int /*scancode*/, int action, int /*mods*/) {
    if (key < 0) return;

    if (action == GLFW_PRESS) {
      state_.setKeyState(static_cast<uint32_t>(key), ButtonState::Pressed);
    } else if (action == GLFW_RELEASE) {
      state_.setKeyState(static_cast<uint32_t>(key), ButtonState::Released);
    }
  }

  void onMouseButton(int button, int action, double x, double y) {
    if (button < 0) return;

    if (action == GLFW_PRESS) {
      state_.setMouseButtonState(static_cast<uint32_t>(button), ButtonState::Pressed);

      if (button == GLFW_MOUSE_BUTTON_LEFT) {
        Pointer p;
        p.startX = p.currentX = static_cast<float>(x);
        p.startY = p.currentY = static_cast<float>(y);
        p.phase = Pointer::Phase::Began;
        state_.AddOrUpdatePointer(kMousePointerId, p);
      }
    } else if (action == GLFW_RELEASE) {
      state_.setMouseButtonState(static_cast<uint32_t>(button), ButtonState::Released);

      if (button == GLFW_MOUSE_BUTTON_LEFT) {
        auto& pointers = state_.pointers();
        auto it = pointers.find(kMousePointerId);
        if (it != pointers.end()) it->second.phase = Pointer::Phase::Ended;
      }
    }
  }

  void onCursorPos(double x, double y) {
    float nx = static_cast<float>(x);
    float ny = static_cast<float>(y);

    state_.setCursorPos(nx, ny);

    auto& pointers = state_.pointers();
    auto it = pointers.find(kMousePointerId);
    if (it == pointers.end()) return;

    Pointer& p = it->second;
    p.deltaX += nx - p.currentX;
    p.deltaY += ny - p.currentY;
    p.currentX = nx;
    p.currentY = ny;
    if (p.phase != Pointer::Phase::Began) p.phase = Pointer::Phase::Moved;
  }

 private:
  static constexpr int32_t kMousePointerId = -1;

  InputState state_;
};

}  // namespace core