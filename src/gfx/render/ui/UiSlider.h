#pragma once

#include "UiBar.h"

namespace gfx {
class UiSlider : public UiBar {
 public:
  [[nodiscard]] float GetMin() const { return min_; }
  void SetMin(float min) { min_ = min; }

  [[nodiscard]] const std::optional<float> GetStride() const { return stride_; }
  void SetStride(const std::optional<float>& stride) { stride_ = stride; }

 private:
  float min_ = 0.0f;
  std::optional<float> stride_ = 0.0f;
};
}  // namespace gfx
