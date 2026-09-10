#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "../../core/InputState.h"
#include "Node.h"

namespace gfx::ui {

struct CameraSwipe {
  float deltaX = 0.0f;
  float deltaY = 0.0f;
};

class InputRouter {
 public:
  CameraSwipe route(const std::unordered_map<int32_t, core::Pointer>& pointers,
                    const std::vector<std::shared_ptr<Node>>& nodes) {
    CameraSwipe swipe = {};

    for (const auto& [id, touch] : pointers) {
      auto it = touchOwner_.find(id);

      if (it == touchOwner_.end()) {
        std::shared_ptr<Node> hit = hitTest(touch.startX, touch.startY, nodes);

        TouchOwner entry;
        entry.node = hit;
        entry.hadTarget = static_cast<bool>(hit);

        it = touchOwner_.emplace(id, std::move(entry)).first;

        if (hit && hit->isInteractive()) {
          hit->press();
        }
      }

      TouchOwner& owner = it->second;

      if (!owner.hadTarget) {
        swipe.deltaX += touch.deltaX;
        swipe.deltaY += touch.deltaY;
      }

      if (touch.phase == core::Pointer::Phase::Ended ||
          touch.phase == core::Pointer::Phase::Cancelled) {
        if (std::shared_ptr<Node> ownerNode = owner.node.lock()) {
          if (ownerNode->isInteractive()) {
            ownerNode->release();
          }
        }

        touchOwner_.erase(it);
      }
    }

    return swipe;
  }

 private:
  struct TouchOwner {
    std::weak_ptr<Node> node;
    bool hadTarget = false;
  };

  std::unordered_map<int32_t, TouchOwner> touchOwner_;

  static std::shared_ptr<Node> hitTest(float x, float y,
                                       const std::vector<std::shared_ptr<Node>>& nodes) {
    for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
      const std::shared_ptr<Node>& node = *it;

      if (!node) {
        continue;
      }

      if (std::shared_ptr<Node> childHit = hitTest(x, y, node->getChildren())) {
        return childHit;
      }

      if (node->contains(x, y) && node->isInteractive()) {
        return node;
      }
    }

    return nullptr;
  }
};

}  // namespace gfx::ui