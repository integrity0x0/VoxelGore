#pragma once

#include <glm/glm.hpp>

namespace gm {
class Environment {
 public:
  void Update(float dt);

  [[nodiscard]] float GetTimeOfDay() const { return timeOfDay_; }
  void SetTimeOfDay(float t);

  [[nodiscard]] glm::vec3 GetSkyWeights() const;
  [[nodiscard]] glm::vec3 GetColor() const;

 private:
  static constexpr float kDayLength = 6.0f * 60.0f;
  float timeOfDay_ = 0.5f;  // 0..1
  float dayLength_ = kDayLength;
  glm::vec3 dayColor_ = glm::vec3(1.0f);
  glm::vec3 sunsetColor_ = glm::vec3(0.85f, 0.5f, 0.4f);
  glm::vec3 nightColor_ = glm::vec3(0.05f, 0.065f, 0.12f);
};

}  // namespace gm