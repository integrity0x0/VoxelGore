#include "Window.h"

namespace core {

Window::Window(uint32_t width, uint32_t height, std::string_view title)
    : window_(glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title.data(),
                               nullptr, nullptr)),
      width_(width),
      height_(height) {
  if (!window_) {
    throw std::runtime_error("failed to create glfw window");
  }

  glfwSetFramebufferSizeCallback(window_.get(), framebufferSizeCallback);
  glfwSetWindowUserPointer(window_.get(), this);
  glfwSetKeyCallback(window_.get(), keyCallback);
  glfwSetMouseButtonCallback(window_.get(), mouseButtonCallback);
  glfwSetCursorPosCallback(window_.get(), cursorPosCallback);
}

void Window::Hint(int32_t hint, int32_t value) { glfwWindowHint(hint, value); }

Window* Window::self(GLFWwindow* w) {
  return reinterpret_cast<Window*>(glfwGetWindowUserPointer(w));
};

void Window::mouseButtonCallback(GLFWwindow* w, int button, int action, int mods) {
  double x, y;
  glfwGetCursorPos(w, &x, &y);
  self(w)->input_.onMouseButton(button, action, x, y);
}

void Window::cursorPosCallback(GLFWwindow* w, double x, double y) {
  self(w)->input_.onCursorPos(x, y);
}

void Window::keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods) {
  self(w)->input_.onKey(key, scancode, action, mods);
}

void Window::framebufferSizeCallback(GLFWwindow* w, int width, int height) {
  auto* window = self(w);
  window->width_ = width;
  window->height_ = height;
  window->resized_ = true;
}
}  // namespace core