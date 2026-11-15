#include "Environment.h"

#include <cmath>

namespace gm {
void Environment::Update(float dt) { timeOfDay_ = std::fmod(timeOfDay_ + dt / dayLength_, 1.0f); }

void Environment::SetTimeOfDay(float t) { timeOfDay_ = t - std::floor(t); }

glm::vec3 Environment::GetSkyWeights() const {
  const float t = timeOfDay_;
  const float sunrise =
      glm::smoothstep(0.20f, 0.30f, t) * (1.0f - glm::smoothstep(0.30f, 0.45f, t));
  const float day = glm::smoothstep(0.30f, 0.45f, t) * (1.0f - glm::smoothstep(0.55f, 0.70f, t));
  const float sunset = glm::smoothstep(0.55f, 0.70f, t) * (1.0f - glm::smoothstep(0.70f, 0.80f, t));
  const float night = 1.0f - (sunrise + day + sunset);
  return glm::vec3(day, sunrise + sunset, night);
}

glm::vec3 Environment::GetColor() const {
  const glm::vec3 w = GetSkyWeights();
  return dayColor_ * w.x + sunsetColor_ * w.y + nightColor_ * w.z;
}

}  // namespace gm