#include "Camera.h"

namespace core {
void Camera::UpdateView() const { view_ = glm::lookAt(pos_, pos_ + forward_, up_); };
}  // namespace core