#include "Camera.h"

namespace core {
void Camera::UpdateView() const {
  const glm::vec3 rolledUp = glm::angleAxis(roll_, forward_) * up_;
  view_ = glm::lookAt(pos_, pos_ + forward_, rolledUp);
  viewDirty_ = false;
}
}  // namespace core