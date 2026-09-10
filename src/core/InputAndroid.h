#pragma once

#include <android/input.h>

#include <cstdint>
#include <unordered_map>

#include "InputState.h"

namespace core {

class InputAndroid {
 public:
  void Update(const AInputEvent* event);

  [[nodiscard]] const InputState& inputState() const { return inputState_; }

  void Reset();

 private:
  void ProcessDown(const AInputEvent* event, int32_t index);
  void ProcessMove(const AInputEvent* event);
  void ProcessUp(const AInputEvent* event, int32_t index);
  void ProcessCancel();

 private:
  InputState inputState_;
};

}  // namespace core