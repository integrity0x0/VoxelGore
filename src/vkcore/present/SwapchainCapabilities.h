#pragma once
#include <vulkan/vulkan.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace vkcore {

struct SwapchainCapabilities {
  VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
  std::vector<VkSurfaceFormatKHR> surfaceFormats;
  std::vector<VkPresentModeKHR> presentModes;

  static SwapchainCapabilities extract(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                       const SurfaceDispatchTable& surfaceTable) {
    SwapchainCapabilities caps;

    SystemError::check(surfaceTable.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                           physicalDevice, surface, &caps.surfaceCapabilities),
                       "SwapchainCapabilities::extract: "
                       "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");

    uint32_t formatCount = 0;
    SystemError::check(surfaceTable.vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
                                                                         &formatCount, nullptr),
                       "SwapchainCapabilities::extract: "
                       "vkGetPhysicalDeviceSurfaceFormatsKHR (count) failed");
    if (formatCount > 0) {
      caps.surfaceFormats.resize(formatCount);
      SystemError::check(surfaceTable.vkGetPhysicalDeviceSurfaceFormatsKHR(
                             physicalDevice, surface, &formatCount, caps.surfaceFormats.data()),
                         "SwapchainCapabilities::extract: "
                         "vkGetPhysicalDeviceSurfaceFormatsKHR failed");
    }

    uint32_t presentModeCount = 0;
    SystemError::check(surfaceTable.vkGetPhysicalDeviceSurfacePresentModesKHR(
                           physicalDevice, surface, &presentModeCount, nullptr),
                       "SwapchainCapabilities::extract: "
                       "vkGetPhysicalDeviceSurfacePresentModesKHR (count) failed");
    if (presentModeCount > 0) {
      caps.presentModes.resize(presentModeCount);
      SystemError::check(surfaceTable.vkGetPhysicalDeviceSurfacePresentModesKHR(
                             physicalDevice, surface, &presentModeCount, caps.presentModes.data()),
                         "SwapchainCapabilities::extract: "
                         "vkGetPhysicalDeviceSurfacePresentModesKHR failed");
    }

    return caps;
  }

  VkSurfaceFormatKHR chooseSurfaceFormat() const {
    for (const auto& format : surfaceFormats) {
      if (format.format == VK_FORMAT_R8G8B8A8_SRGB &&
          format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return format;
      }
    }
    if (!surfaceFormats.empty()) {
      return surfaceFormats[0];
    }
    throw std::runtime_error(
        "SwapchainCapabilities::chooseSurfaceFormat: no "
        "surface formats available");
  }

  VkPresentModeKHR choosePresentMode() const {
    for (const auto& mode : presentModes) {
      if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return mode;
      }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D chooseExtent(uint32_t desiredWidth, uint32_t desiredHeight) const {
    if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
      return surfaceCapabilities.currentExtent;
    }

    VkExtent2D actualExtent = {desiredWidth, desiredHeight};

    actualExtent.width = std::clamp(actualExtent.width, surfaceCapabilities.minImageExtent.width,
                                    surfaceCapabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, surfaceCapabilities.minImageExtent.height,
                                     surfaceCapabilities.maxImageExtent.height);

    return actualExtent;
  }

  uint32_t chooseImageCount() const {
    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
      imageCount = surfaceCapabilities.maxImageCount;
    }
    return imageCount;
  }

  VkCompositeAlphaFlagBitsKHR chooseCompositeAlpha() const {
    if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
      return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }
    if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
      return VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
    }
    if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
      return VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    }
    if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
      return VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }
    throw std::runtime_error(
        "SwapchainCapabilities::chooseCompositeAlpha: no "
        "supported composite alpha");
  }

  VkImageUsageFlags chooseImageUsage() const {
    VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if ((surfaceCapabilities.supportedUsageFlags & usage) == 0) {
      throw std::runtime_error(
          "SwapchainCapabilities::chooseImageUsage: "
          "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT not supported");
    }
    return usage;
  }
};

}  // namespace vkcore