#pragma once

#include <memory>
#include <vector>

#include "Node.h"

namespace gfx::ui {
class Container : public Node {
 public:
  void addChild(std::shared_ptr<Node> child) {
    // child->setParent(this);
    children_.push_back(std::move(child));
  }

  const std::vector<std::shared_ptr<Node>>& getChildren() const override { return children_; }

  void Render(Renderer& renderer) const override;

 private:
  std::vector<std::shared_ptr<Node>> children_;
};
}  // namespace gfx::ui