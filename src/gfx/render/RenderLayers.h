#pragma once

namespace gfx {
enum class RenderLayer { Solid, Cutout, Translucent, Count };

template <typename T>
RenderLayer ToRenderLayer(T value) {
  return static_cast<RenderLayer>(value);
}

}  // namespace gfx