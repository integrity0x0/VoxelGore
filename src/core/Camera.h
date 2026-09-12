#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace core {

class Camera {
 public:
  void Rotate(float dx, float dy, float sensitivity = 0.1f) {
    yaw += dx * sensitivity;
    pitch -= dy * sensitivity;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
  }

  void Move(float forwardAxis, float rightAxis, float speed, float dt) {
    glm::vec3 forward = GetForward();
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    position += (forward * forwardAxis + right * rightAxis) * speed * dt;
  }

  glm::vec3 GetForward() const {
    return glm::normalize(glm::vec3(cosf(glm::radians(yaw)) * cosf(glm::radians(pitch)),
                                    sinf(glm::radians(pitch)),
                                    sinf(glm::radians(yaw)) * cosf(glm::radians(pitch))));
  }

  void LookAt(const glm::vec3& eye, const glm::vec3& target) {
    position = eye;
    glm::vec3 dir = glm::normalize(target - eye);
    pitch = glm::degrees(asinf(dir.y));
    yaw = glm::degrees(atan2f(dir.x, dir.z));
  }

  glm::mat4 GetView() const { return glm::lookAt(position, position + GetForward(), up); }

  glm::vec3 position{0.0f, 1.0f, -3.0f};
  glm::vec3 up{0.0f, 1.0f, 0.0f};
  float yaw = 0.0f;
  float pitch = 0.0f;
};

}  // namespace core