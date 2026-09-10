#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "InputGlfw.h"

namespace core {

struct GLFWwindowDeleter {
  void operator()(GLFWwindow* window) const {
    if (window) {
      glfwDestroyWindow(window);
    }
  }
};

class Window {
 public:
  Window(uint32_t width, uint32_t height, std::string_view title);
  void Update() {
    input_.endFrame();
    resized_ = false;
  };
  static void hint(int32_t hint, int32_t value);

  [[nodiscard]] GLFWwindow* getWindow() const { return window_.get(); }
  [[nodiscard]] InputGlfw& getInput() { return input_; }
  [[nodiscard]] const InputGlfw& getInput() const { return input_; }
  [[nodiscard]] uint32_t width() const { return width_; }
  [[nodiscard]] uint32_t height() const { return height_; }
  [[nodiscard]] float getAspectRatio() const {
    return height_ ? static_cast<float>(width_) / static_cast<float>(height_) : 0.0f;
  }

  void setSize(uint32_t width, uint32_t height) {
    width_ = width;
    height_ = height;
    glfwSetWindowSize(window_.get(), static_cast<int>(width), static_cast<int>(height));
  }

  void setTitle(std::string_view title) { glfwSetWindowTitle(window_.get(), title.data()); }

  [[nodiscard]] bool shouldClose() const { return glfwWindowShouldClose(window_.get()); }

  void setShouldClose(bool close) { glfwSetWindowShouldClose(window_.get(), close); }

  [[nodiscard]] bool isMinimized() const {
    return glfwGetWindowAttrib(window_.get(), GLFW_ICONIFIED);
  }

  [[nodiscard]] bool isMaximized() const {
    return glfwGetWindowAttrib(window_.get(), GLFW_MAXIMIZED);
  }

  [[nodiscard]] bool isFocused() const { return glfwGetWindowAttrib(window_.get(), GLFW_FOCUSED); }

  void maximize() { glfwMaximizeWindow(window_.get()); }

  void minimize() { glfwIconifyWindow(window_.get()); }

  void restore() { glfwRestoreWindow(window_.get()); }

  void hide() { glfwHideWindow(window_.get()); }

  void show() { glfwShowWindow(window_.get()); }

  void setPosition(int32_t x, int32_t y) { glfwSetWindowPos(window_.get(), x, y); }

  [[nodiscard]] std::pair<int32_t, int32_t> getPosition() const {
    int32_t x = 0, y = 0;
    glfwGetWindowPos(window_.get(), &x, &y);
    return {x, y};
  }

  [[nodiscard]] bool isResized() const { return resized_; }

 private:
  std::unique_ptr<GLFWwindow, GLFWwindowDeleter> window_;
  InputGlfw input_;
  uint32_t width_, height_;
  bool resized_ = false;

  [[nodiscard]] static Window* self(GLFWwindow* w);

  static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
  static void mouseButtonCallback(GLFWwindow* w, int button, int action, int mods);
  static void cursorPosCallback(GLFWwindow* w, double x, double y);
  static void keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods);
};

}  // namespace core