#pragma once

#include <tinyxml2.h>

#include <glm/glm.hpp>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

#include "InputRouter.h"
#include "Page.h"
#include "Renderer.h"

namespace gfx::ui {
class Ui {
 public:
  Ui(const vkcore::Device& device, const vkcore::CommandPool& commandPool, VkRenderPass renderPass,
     const script::LuaState& luaState, TextureManager& textureManager, VkExtent2D initialExtent)
      : device_(&device),
        commandPool_(&commandPool),
        luaState_(&luaState),
        textureManager_(&textureManager),
        renderer_(device, renderPass, textureManager) {
    resize(initialExtent);
  }

  Page& push(std::string_view xmlPath) {
    std::string id(xmlPath);

    auto it = pages_.find(id);
    if (it == pages_.end()) {
      try {
        it = pages_.try_emplace(id, *device_, *commandPool_, *luaState_, xmlPath).first;
      } catch (...) {
        pages_.erase(id);
        throw;
      }
    }

    pageStack_.push(&it->second);
    return it->second;
  }

  void resize(VkExtent2D extent) {
    screenSize_ = {static_cast<float>(extent.width), static_cast<float>(extent.height)};
    renderer_.setScreenSize(screenSize_);
  }

  void pop() {
    if (pageStack_.size() <= 1) {
      throw std::runtime_error("Cannot pop the last page off the stack");
    }
    pageStack_.pop();
  }

  [[nodiscard]] Page& top() {
    if (pageStack_.empty()) throw std::runtime_error("Page stack is empty");
    return *pageStack_.top();
  }

  const Page& top() const {
    if (pageStack_.empty()) throw std::runtime_error("Page stack is empty");
    return *pageStack_.top();
  }

  bool empty() const { return pageStack_.empty(); }
  size_t depth() const { return pageStack_.size(); }

  CameraSwipe routeTouches(const std::unordered_map<int32_t, core::Pointer>& pointers) {
    if (pageStack_.empty()) return CameraSwipe{};
    return inputRouter_.route(pointers, top().getNodes());
  }

  void Update() {
    if (!pageStack_.empty()) pageStack_.top()->Update(screenSize_);
  }

  void render(VkCommandBuffer cmd) {
    renderer_.begin(cmd);
    if (!pageStack_.empty()) pageStack_.top()->render(renderer_);
    renderer_.end();
  }

 private:
  const vkcore::Device* device_;
  const vkcore::CommandPool* commandPool_;
  const script::LuaState* luaState_;
  TextureManager* textureManager_;
  Renderer renderer_;
  InputRouter inputRouter_;
  glm::vec2 screenSize_{1.0f, 1.0f};

  std::unordered_map<std::string, Page> pages_;
  std::stack<Page*> pageStack_;
};
}  // namespace gfx::ui