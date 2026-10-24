#pragma once

#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>

namespace core {

class CameraShake {
 public:
  void Update(float dt, glm::vec3 horizontalVelocity, bool grounded);

  void AddTrauma(float amount, float maxTrauma) {
    if (trauma < maxTrauma) trauma = std::min(maxTrauma, trauma + amount);
  }

  void Impulse(glm::vec3 direction, float strength, float trauma = 0.0f);

  [[nodiscard]] glm::vec3 GetPositionOffset() const;
  [[nodiscard]] float GetRoll() const;

 private:

  float bobPhase = 0.0f;
  float intensity = 0.0f;
  float airTime = 0.0f;
  float bobFrequency = 1.25f;
  float bobAmplitudeY = 0.05f;
  float bobAmplitudeX = 0.03f;

  glm::vec3 kickOffset{0.0f};
  glm::vec3 kickVelocity{0.0f};
  float roll = 0.0f;
  float rollVelocity = 0.0f;
  float springStiffness = 150.0f;
  float springDamping = 12.0f;

  float trauma = 0.0f;
  float traumaDecay = 1.5f;
  float shakeTime = 0.0f;
  float shakeAmplitude = 0.08f;
};

}  // namespace core