#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>

#define KEY_LAST 349
#define MOUSE_BUTTON_LAST 8

namespace core {

struct Pointer {
  float startX = 0.0f, startY = 0.0f;
  float currentX = 0.0f, currentY = 0.0f;
  float prevX = 0.0f, prevY = 0.0f;
  float deltaX = 0.0f, deltaY = 0.0f;
  float pressure = 0.0f;

  enum class Phase { None, Began, Moved, Ended, Cancelled } phase = Phase::None;
};

enum class ButtonState { Up, Pressed, Held, Released };

class InputState {
 public:
  InputState();

  [[nodiscard]] const std::unordered_map<int32_t, Pointer>& pointers() const { return pointers_; }
  [[nodiscard]] std::unordered_map<int32_t, Pointer>& pointers() { return pointers_; }

  [[nodiscard]] bool pressed(uint32_t keyCode) const {
    return IsInState(keyStates_, keyCode, ButtonState::Pressed);
  }
  [[nodiscard]] bool Down(uint32_t keyCode) const { return IsDownOrHeld(keyStates_, keyCode); }
  [[nodiscard]] bool Released(uint32_t keyCode) const {
    return IsInState(keyStates_, keyCode, ButtonState::Released);
  }

  [[nodiscard]] bool MousePressed(uint32_t buttonIndex) const {
    return IsInState(mouseButtonStates_, buttonIndex, ButtonState::Pressed);
  }
  [[nodiscard]] bool MouseDown(uint32_t buttonIndex) const {
    return IsDownOrHeld(mouseButtonStates_, buttonIndex);
  }
  [[nodiscard]] bool MouseReleased(uint32_t buttonIndex) const {
    return IsInState(mouseButtonStates_, buttonIndex, ButtonState::Released);
  }

  void setKeyState(uint32_t keyCode, ButtonState state);
  void setMouseButtonState(uint32_t buttonIndex, ButtonState state);

  void AddOrUpdatePointer(int32_t pointerId, const Pointer& ptr) { pointers_[pointerId] = ptr; }

  void RemovePointer(int32_t pointerId) { pointers_.erase(pointerId); }

  void setCursorPos(float x, float y) {
    cursorDeltaX_ += x - cursorPosX_;
    cursorDeltaY_ += y - cursorPosY_;
    cursorPosX_ = x;
    cursorPosY_ = y;
  }

  [[nodiscard]] float cursorX() const { return cursorPosX_; }
  [[nodiscard]] float cursorY() const { return cursorPosY_; }
  [[nodiscard]] float cursorDeltaX() const { return cursorDeltaX_; }
  [[nodiscard]] float cursorDeltaY() const { return cursorDeltaY_; }

  void Reset();

 private:
  template <size_t N>
  [[nodiscard]] bool IsInState(const std::array<ButtonState, N>& arr, uint32_t index,
                               ButtonState target) const {
    if (index >= N) return false;
    return arr[index] == target;
  }

  template <size_t N>
  [[nodiscard]] bool IsDownOrHeld(const std::array<ButtonState, N>& arr, uint32_t index) const {
    if (index >= N) return false;
    auto s = arr[index];
    return s == ButtonState::Pressed || s == ButtonState::Held;
  }

  template <size_t N>
  static void AdvanceStates(std::array<ButtonState, N>& arr) {
    for (auto& s : arr) {
      if (s == ButtonState::Pressed)
        s = ButtonState::Held;
      else if (s == ButtonState::Released)
        s = ButtonState::Up;
    }
  }

  std::array<ButtonState, KEY_LAST + 1u> keyStates_;
  std::array<ButtonState, MOUSE_BUTTON_LAST + 1u> mouseButtonStates_;

  std::unordered_map<int32_t, Pointer> pointers_;

  float cursorPosX_ = 0.0f, cursorPosY_ = 0.0f;
  float cursorDeltaX_ = 0.0f, cursorDeltaY_ = 0.0f;
};

}  // namespace core