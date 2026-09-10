#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "../devices/Device.h"
#include "../resource/ImageView.h"
#include "Surface.h"
#include "SwapchainCapabilities.h"

namespace vkcore {

class Swapchain {
 public:
  Swapchain(const Device& device, const Surface& surface);

  VkSwapchainKHR handle() const noexcept { return swapchain_.get(); }
  VkFormat getImageFormat() const noexcept { return imageFormat_; }
  VkExtent2D extent() const noexcept { return extent_; }
  uint32_t getImageCount() const noexcept { return static_cast<uint32_t>(images_.size()); }
  const std::vector<VkImage>& getImages() const noexcept { return images_; }
  const std::vector<ImageView>& getImageViews() const noexcept { return imageViews_; }

 private:
  const Device* device_;
  UniqueSwapchainKHR swapchain_;
  std::vector<VkImage> images_;
  std::vector<ImageView> imageViews_;
  VkFormat imageFormat_ = VK_FORMAT_UNDEFINED;
  VkExtent2D extent_ = {0, 0};
};

}  // namespace vkcore