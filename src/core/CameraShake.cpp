#include "CameraShake.h"

#include <cmath>

namespace core {

void CameraShake::Update(float dt, glm::vec3 horizontalVelocity, bool grounded) {
  float speed = glm::length(horizontalVelocity);
  bool walking = grounded && speed > 0.1f;

  if (walking) {
    distanceTraveled += speed * dt;
  }

  float target = walking ? 1.0f : 0.0f;
  intensity = glm::mix(intensity, target, glm::clamp(dt * 8.0f, 0.0f, 1.0f));
}

glm::vec3 CameraShake::getPositionOffset() const {
  float phase = distanceTraveled * bobFrequency;

  float vertical = sinf(phase * 2.0f) * bobAmplitudeY;
  float horizontal = cosf(phase) * bobAmplitudeX;

  return glm::vec3(horizontal, vertical, 0.0f) * intensity;
}

}  // namespace core