#include "CameraShake.h"

#include <algorithm>
#include <cmath>

namespace core {

namespace {
constexpr float kTwoPi = 6.28318530718f;

float Noise(float t, float seed) {
  return 0.6f * sinf(t * 90.0f + seed) + 0.4f * sinf(t * 140.0f + seed * 1.7f);
}
}  // namespace

void CameraShake::Update(float dt, glm::vec3 horizontalVelocity, bool grounded) {
  dt = std::min(dt, 0.05f);

  float speed = glm::length(horizontalVelocity);
  airTime = grounded ? 0.0f : airTime + dt;
  bool walking = airTime < 0.1f && speed > 0.1f;

  if (walking) {
    bobPhase = fmodf(bobPhase + speed * dt * bobFrequency, kTwoPi);
  }
  float target = walking ? 1.0f : 0.0f;
  intensity += (target - intensity) * (1.0f - expf(-8.0f * dt));

  glm::vec3 acc = -kickOffset * springStiffness - kickVelocity * springDamping;
  kickVelocity += acc * dt;
  kickOffset += kickVelocity * dt;

  float rollAcc = -roll * springStiffness - rollVelocity * springDamping;
  rollVelocity += rollAcc * dt;
  roll += rollVelocity * dt;

  trauma = std::max(0.0f, trauma - traumaDecay * dt);
  shakeTime += dt;
}

void CameraShake::Impulse(glm::vec3 direction, float strength, float traumaAmount) {
  if (glm::length(direction) > 1e-4f) {
    direction = glm::normalize(direction);
  }
  kickVelocity += direction * strength;
  rollVelocity += -direction.x * strength * 0.3f;
  trauma = std::min(1.0f, trauma + traumaAmount);
}

glm::vec3 CameraShake::GetPositionOffset() const {
  float vertical = sinf(bobPhase * 2.0f) * bobAmplitudeY;
  float horizontal = cosf(bobPhase) * bobAmplitudeX;
  glm::vec3 bob = glm::vec3(horizontal, vertical, 0.0f) * intensity;

  float t2 = trauma * trauma;
  glm::vec3 shake(Noise(shakeTime, 0.0f), Noise(shakeTime, 17.0f), 0.0f);
  shake *= t2 * shakeAmplitude;

  return bob + kickOffset + shake;
}

float CameraShake::GetRoll() const {
  float t2 = trauma * trauma;
  return roll + Noise(shakeTime, 41.0f) * t2 * 0.04f;
}

}  // namespace core