#pragma once

#include "../../common/texture/Skybox.h"

namespace gfx {
class Sky {
 public:
  [[nodiscard]] std::optional<Sky> Load();
 private:
  Skybox day_;
  Skybox sunset_;
  Skybox night_;
};
}  // namespace gfx
