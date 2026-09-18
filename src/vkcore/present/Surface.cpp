#include "Surface.h"

namespace vkcore {

#ifdef VK_USE_PLATFORM_ANDROID_KHR

BlockSurface::BlockSurface(const Instance& instance, ANativeWindow* window) : instance_(&instance) {
  if (!window) {
    throw std::runtime_error("Surface: ANativeWindow* is null");
  }

  const auto& dispatchTable = instance_->getDispatchTable();

  if (!dispatchTable.androidSurfaceTable.has_value()) {
    throw std::runtime_error("Surface: AndroidSurfaceDispatchTable not loaded");
  }

  const auto& androidTable = dispatchTable.androidSurfaceTable.value();
  if (!androidTable.vkCreateAndroidSurfaceKHR) {
    throw std::runtime_error("Surface: vkCreateAndroidSurfaceKHR is null");
  }

  VkAndroidSurfaceCreateInfoKHR surfaceCI = {VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
  surfaceCI.window = window;

  VkSurfaceKHR surfaceRaw = VK_NULL_HANDLE;
  VkResult result =
      androidTable.vkCreateAndroidSurfaceKHR(instance_->handle(), &surfaceCI, nullptr, &surfaceRaw);
  SystemError::Check(result, "Surface: vkCreateAndroidSurfaceKHR failed");

  setupDeleter(surfaceRaw);
}

#else

Surface::Surface(const Instance& instance, GLFWwindow* window) : instance_(&instance) {
  if (!window) {
    throw std::runtime_error("Surface: GLFWwindow* is null");
  }

  VkSurfaceKHR surfaceRaw = VK_NULL_HANDLE;
  VkResult result = glfwCreateWindowSurface(instance_->handle(), window, nullptr, &surfaceRaw);
  SystemError::Check(result, "Surface: glfwCreateWindowSurface failed");

  setupDeleter(surfaceRaw);
}

#endif

void Surface::setupDeleter(VkSurfaceKHR surfaceRaw) {
  const auto& dispatchTable = instance_->getDispatchTable();

  if (!dispatchTable.surfaceTable.has_value()) {
    throw std::runtime_error("Surface: SurfaceDispatchTable not loaded");
  }

  const auto& surfaceTable = dispatchTable.surfaceTable.value();

  SurfaceDeleter deleter = {};
  deleter.instance = instance_->handle();
  deleter.func = surfaceTable.vkDestroySurfaceKHR;

  surface_ = UniqueSurfaceKHR(surfaceRaw, deleter);
}

VkSurfaceKHR Surface::handle() const noexcept { return surface_.get(); }

const Instance& Surface::getInstance() const noexcept { return *instance_; }

}  // namespace vkcore