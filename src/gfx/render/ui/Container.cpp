#include "Container.h"

namespace gfx::ui {

void Container::Render(Renderer& renderer) const {
  Node::Render(renderer);

  for (const auto& child : children_) {
    child->Render(renderer);
  }
}

}  // namespace gfx::ui