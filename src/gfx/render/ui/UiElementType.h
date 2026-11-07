#pragma once

#include <stdint.h>

namespace gfx {
enum class UiElementType : uint8_t {
  Undefined,
  Container,
  Text,
  Image,
  Button,
  Bar,
  Checkbox,
  Slider,
  Stick,
  Grid
};
}  // namespace gfx