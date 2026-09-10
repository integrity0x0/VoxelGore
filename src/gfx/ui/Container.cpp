#include "Container.h"

namespace gfx::ui {

void Container::render(Renderer& renderer) const {
  Node::render(renderer);

  for (const auto& child : children_) {
    child->render(renderer);
  }
}

}  // namespace gfx::ui