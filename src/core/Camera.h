#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace core {

class Camera {
 public:
  void Rotate(float dx, float dy, float sensitivity = 0.1f) {
    yaw_ += dx * sensitivity;
    pitch_ -= dy * sensitivity;
    pitch_ = glm::clamp(pitch_, -89.0f, 89.0f);

    forward_ = glm::normalize(glm::vec3(cosf(glm::radians(yaw_)) * cosf(glm::radians(pitch_)),
                                        sinf(glm::radians(pitch_)),
                                        sinf(glm::radians(yaw_)) * cosf(glm::radians(pitch_))));
    right_ = glm::normalize(glm::cross(forward_, up_));
    viewDirty_ = true;
  }

  void Move(float forwardAxis, float rightAxis, float speed, float dt) {   
    pos_ += (forward_ * forwardAxis + right_ * rightAxis) * speed * dt;
    viewDirty_ = true;
  }

  void SetPos(const glm::vec3& pos) {
    pos_ = pos;
    viewDirty_ = true;
  }

  [[nodiscard]] const glm::vec3& pos() const {
    return pos_;
  }

  [[nodiscard]] const glm::vec3& forward() const {
    return forward_;
  }

  [[nodiscard]] const glm::vec3& right() const {
    return right_;
  }

  [[nodiscard]] const glm::vec3& up() const { return up_; }

  [[nodiscard]] float yaw() const { return yaw_; }
  [[nodiscard]] float pitch() const { return pitch_; }

  void LookAt(const glm::vec3& eye, const glm::vec3& target) {
    pos_ = eye;
    forward_ = glm::normalize(target - eye);
    pitch_ = glm::degrees(asinf(forward_.y));
    yaw_ = glm::degrees(atan2f(forward_.x, forward_.z));
    viewDirty_ = true;
  }

  [[nodiscard]] glm::mat4 view() const { 
    if (viewDirty_) UpdateView(); 
    return view_; 
  }

 private:
  void UpdateView() const;
 private:

  glm::vec3 pos_ = {0.0f, 1.0f, -3.0f};
  glm::vec3 up_ = {0.0f, 1.0f, 0.0f};
  float yaw_ = 0.0f;
  float pitch_ = 0.0f;
  glm::vec3 forward_ = {};
  glm::vec3 right_ = {};
  mutable glm::mat4 view_ = glm::mat4(1.0f);
  mutable bool viewDirty_ = true;
};

}  // namespace core