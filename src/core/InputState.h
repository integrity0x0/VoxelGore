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
  static constexpr int32_t kMousePointerId = 32;
  static constexpr int32_t kMaxPointers = kMousePointerId + 1;

  InputState();

  [[nodiscard]] const std::unordered_map<int32_t, Pointer>& GetPointers() const { return pointers_; }
  [[nodiscard]] std::unordered_map<int32_t, Pointer>& GetPointers() { return pointers_; }

  [[nodiscard]] bool Pressed(uint32_t keyCode) const {
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

  void SetKeyState(uint32_t keyCode, ButtonState state);
  void SetMouseButtonState(uint32_t buttonIndex, ButtonState state);

  void AddOrUpdatePointer(int32_t pointerId, const Pointer& ptr) { pointers_[pointerId] = ptr; }

  void RemovePointer(int32_t pointerId) { pointers_.erase(pointerId); }

  void SetCursorPos(float x, float y) {
    if (baselineValid_) {
      cursorDeltaX_ += x - cursorPosX_;
      cursorDeltaY_ += y - cursorPosY_;
    }
    baselineValid_ = true;
    hasCursor_ = true;
    cursorPosX_ = x;
    cursorPosY_ = y;
  }

  void SetCursorDisabled(bool disabled) {
    if (cursorDisabled_ == disabled) return;
    cursorDisabled_ = disabled;
    baselineValid_ = false;
    cursorDeltaX_ = cursorDeltaY_ = 0.0f;

    if (disabled) {
      if (auto it = pointers_.find(kMousePointerId); it != pointers_.end()) {
        it->second.phase = Pointer::Phase::Cancelled;
      }
    }
  }

  [[nodiscard]] float GetCursorX() const { return cursorPosX_; }
  [[nodiscard]] float GetCursorY() const { return cursorPosY_; }
  [[nodiscard]] float GetCursorDeltaX() const { return cursorDeltaX_; }
  [[nodiscard]] float GetCursorDeltaY() const { return cursorDeltaY_; }
  [[nodiscard]] bool HasCursor() const { return hasCursor_; }
  [[nodiscard]] bool IsCursorDisabled() const { return cursorDisabled_; }
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

  bool hasCursor_ = false;
  float cursorPosX_ = 0.0f, cursorPosY_ = 0.0f;
  float cursorDeltaX_ = 0.0f, cursorDeltaY_ = 0.0f;
  bool cursorDisabled_ = false;
  bool baselineValid_ = false;
};

}  // namespace core