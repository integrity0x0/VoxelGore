#pragma once

#include <glm/glm.hpp>

namespace gm {
class Enviroment {
 public:
  void Update(float dt);

  [[nodiscard]] glm::vec3 GetColor() const {
    return glm::mix(nightColor_, dayColor_, fmod(timeOfDay_, 0.5f));
  }

 private:
  static constexpr float kDayLength = 24.0f * 60.0f;
  float timeOfDay_ = 0.0f;
  float dayLength_ = kDayLength;
  glm::vec3 ambientColor_;
  glm::vec3 nightColor_ = glm::vec3(0.05f, 0.065f, 0.12f);
  glm::vec3 dayColor_ = glm::vec3(0.0f);
};

}  // namespace gm