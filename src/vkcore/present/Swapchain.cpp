#include "Swapchain.h"

namespace vkcore {

Swapchain::Swapchain(const Device& device, const Surface& surface) : device_(&device) {
  const auto& dispatchTable = device.dispatchTable();
  const auto& instanceDispatchTable = surface.getInstance().dispatchTable();

  if (!instanceDispatchTable.surfaceTable.has_value()) {
    throw std::runtime_error("Swapchain: SurfaceDispatchTable not loaded");
  }
  const auto& surfaceTable = instanceDispatchTable.surfaceTable.value();

  if (!dispatchTable.swapchainTable.has_value()) {
    throw std::runtime_error("Swapchain: SwapchainDispatchTable not loaded");
  }
  const auto& swapchainTable = dispatchTable.swapchainTable.value();

  SwapchainCapabilities caps = SwapchainCapabilities::Build(device.getPhysicalDevice().handle(),
                                                              surface.handle(), surfaceTable);

  VkSurfaceFormatKHR chosenFormat = caps.ChooseSurfaceFormat();
  VkPresentModeKHR chosenPresentMode = caps.ChoosePresentMode();
  extent_ = caps.ChooseExtent(0, 0);
  uint32_t imageCount = caps.ChooseImageCount();
  VkCompositeAlphaFlagBitsKHR compositeAlpha = caps.ChooseCompositeAlpha();
  VkImageUsageFlags imageUsage = caps.ChooseImageUsage();
  imageFormat_ = chosenFormat.format;

  VkSwapchainCreateInfoKHR swapchainCI = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  swapchainCI.surface = surface.handle();
  swapchainCI.minImageCount = imageCount;
  swapchainCI.imageFormat = chosenFormat.format;
  swapchainCI.imageColorSpace = chosenFormat.colorSpace;
  swapchainCI.imageExtent = extent_;
  swapchainCI.imageArrayLayers = 1;
  swapchainCI.imageUsage = imageUsage;
  swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchainCI.compositeAlpha = compositeAlpha;
  swapchainCI.presentMode = chosenPresentMode;
  swapchainCI.clipped = VK_TRUE;
  swapchainCI.oldSwapchain = VK_NULL_HANDLE;

  VkSwapchainKHR rawSwapchain = VK_NULL_HANDLE;
  SystemError::Check(
      swapchainTable.vkCreateSwapchainKHR(device.handle(), &swapchainCI, nullptr, &rawSwapchain),
      "Swapchain: vkCreateSwapchainKHR failed");

  SwapchainDeleter deleter;
  deleter.device = device.handle();
  deleter.func = swapchainTable.vkDestroySwapchainKHR;
  swapchain_ = UniqueSwapchainKHR(rawSwapchain, deleter);

  uint32_t actualImageCount = 0;
  SystemError::Check(swapchainTable.vkGetSwapchainImagesKHR(device.handle(), rawSwapchain,
                                                            &actualImageCount, nullptr),
                     "Swapchain: vkGetSwapchainImagesKHR (count) failed");
  images_.resize(actualImageCount);
  SystemError::Check(swapchainTable.vkGetSwapchainImagesKHR(device.handle(), rawSwapchain,
                                                            &actualImageCount, images_.data()),
                     "Swapchain: vkGetSwapchainImagesKHR failed");

  imageViews_.reserve(images_.size());
  for (size_t i = 0; i < images_.size(); ++i) {
    VkImageViewCreateInfo imageViewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    imageViewCI.image = images_[i];
    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCI.format = imageFormat_;
    imageViewCI.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                              VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    imageViewCI.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    imageViews_.emplace_back(device, imageViewCI);
  }
}

}  // namespace vkcore