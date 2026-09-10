#pragma once
#include <glm/glm.hpp>

namespace core {

class CameraShake {
 public:
  void Update(float dt, glm::vec3 horizontalVelocity, bool grounded);
  [[nodiscard]] glm::vec3 getPositionOffset() const;

 private:
  float distanceTraveled = 0.0f;
  float intensity = 0.0f;

  float bobFrequency = 1.8f;
  float bobAmplitudeY = 0.05f;
  float bobAmplitudeX = 0.03f;
};

}  // namespace core