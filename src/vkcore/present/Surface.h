#pragma once

#include "../instances/Instance.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include <android/native_window.h>
#else
#include <GLFW/glfw3.h>
#endif

namespace vkcore {

class Surface {
 public:
#ifdef VK_USE_PLATFORM_ANDROID_KHR
  BlockSurface(const Instance& instance, ANativeWindow* window);
#else
  Surface(const Instance& instance, GLFWwindow* window);
#endif
  Surface(const Instance& instance, UniqueSurfaceKHR&& surface)
      : instance_(&instance), surface_(std::move(surface)) {}

  [[nodiscard]] VkSurfaceKHR handle() const noexcept;
  [[nodiscard]] const Instance& getInstance() const noexcept;

 private:
  void setupDeleter(VkSurfaceKHR surfaceRaw);

  const Instance* instance_;
  UniqueSurfaceKHR surface_;
};

}  // namespace vkcore