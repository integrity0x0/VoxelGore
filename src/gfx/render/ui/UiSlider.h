#pragma once

#include "UiBar.h"

namespace gfx {
class UiSlider : public UiBar {
 public:
  [[nodiscard]] float GetMin() const { return min_; }
  void SetFloat(float min) { min_ = min; }

  [[nodiscard]] float GetStride() const { return stride_; }
  void SetStride(float stride) { stride_ = stride; }

 private:
  float min_ = 0.0f;
  float stride_ = 0.0f;
};
}  // namespace gfx
