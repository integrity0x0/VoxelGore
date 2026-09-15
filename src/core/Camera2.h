#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace core {

class Camera2 {
 public:
  void Rotate(float dx, float dy, float sensitivity = 0.1f) {
    yaw_ += dx * sensitivity;
    pitch_ -= dy * sensitivity;
    pitch_ = glm::clamp(pitch_, -89.0f, 89.0f);
    viewDirty_ = true;
  }

  void Move(float forwardAxis, float rightAxis, float speed, float dt) {
    glm::vec3 forward = GetForward();
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    position += (forward * forwardAxis + right * rightAxis) * speed * dt;
    viewDirty_ = true;
  }

  glm::vec3 GetForward() const {
    return glm::normalize(glm::vec3(cosf(glm::radians(yaw_)) * cosf(glm::radians(pitch_)),
                                    sinf(glm::radians(pitch_)),
                                    sinf(glm::radians(yaw_)) * cosf(glm::radians(pitch_))));
  }

  void LookAt(const glm::vec3& eye, const glm::vec3& target) {
    pos_ = eye;
    glm::vec3 dir = glm::normalize(target - eye);
    pitch_ = glm::degrees(asinf(dir.y));
    yaw_ = glm::degrees(atan2f(dir.x, dir.z));
    viewDirty_ = true;
  }

  glm::mat4 GetView() const { 
    if (viewDirty_) {
      UpdateView();
    } 
    return view_; 
  }

 private:
  void UpdateView() const;
 private:

  glm::vec3 pos_ = {0.0f, 1.0f, -3.0f};
  glm::vec3 up_ = {0.0f, 1.0f, 0.0f};
  float yaw_ = 0.0f;
  float pitch_ = 0.0f;

  mutable glm::mat4 view_;
  mutable bool viewDirty_ = true;
};

}  // namespace core