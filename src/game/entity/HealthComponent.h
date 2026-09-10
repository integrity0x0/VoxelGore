#pragma once

#include <algorithm>

namespace gm {

struct HealthComponent {
  float current = 100.0f;
  float max = 100.0f;

  static constexpr float kHurtFlashDuration = 0.25f;

  float hurtFlash = 0.0f;

  bool isDead() const { return current <= 0.0f; }

  void Damage(float amount) {
    current = std::max(0.0f, current - amount);
    hurtFlash = kHurtFlashDuration;
  }

  void Heal(float amount) { current = std::min(max, current + amount); }
};

}  // namespace gm