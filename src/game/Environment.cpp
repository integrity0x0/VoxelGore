#include "Environment.h"

#include <cmath>

namespace gm {
void Environment::Update(float dt) {
  timeOfDay_ += std::fmod(timeOfDay_ + dt / dayLength_, 1.0f);
}
}  // namespace gm