#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace vkcore {

struct SwapchainCapabilities {
  VkSurfaceCapabilitiesKHR surfaceCapabilities{};
  std::vector<VkSurfaceFormatKHR> surfaceFormats;
  std::vector<VkPresentModeKHR> presentModes;

  [[nodiscard]] static SwapchainCapabilities Build(VkPhysicalDevice physicalDevice,
                                                     VkSurfaceKHR surface,
                                                     const SurfaceDispatchTable& surfaceTable) {
    SwapchainCapabilities caps;

    SystemError::Check(surfaceTable.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                           physicalDevice, surface, &caps.surfaceCapabilities),
                       "SwapchainCapabilities::Build: "
                       "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");

    uint32_t formatCount = 0;
    SystemError::Check(surfaceTable.vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
                                                                         &formatCount, nullptr),
                       "SwapchainCapabilities::Build: "
                       "vkGetPhysicalDeviceSurfaceFormatsKHR (count) failed");

    if (formatCount > 0) {
      caps.surfaceFormats.resize(formatCount);

      SystemError::Check(surfaceTable.vkGetPhysicalDeviceSurfaceFormatsKHR(
                             physicalDevice, surface, &formatCount, caps.surfaceFormats.data()),
                         "SwapchainCapabilities::Build: "
                         "vkGetPhysicalDeviceSurfaceFormatsKHR failed");
    }

    uint32_t presentModeCount = 0;
    SystemError::Check(surfaceTable.vkGetPhysicalDeviceSurfacePresentModesKHR(
                           physicalDevice, surface, &presentModeCount, nullptr),
                       "SwapchainCapabilities::Build: "
                       "vkGetPhysicalDeviceSurfacePresentModesKHR (count) failed");

    if (presentModeCount > 0) {
      caps.presentModes.resize(presentModeCount);

      SystemError::Check(surfaceTable.vkGetPhysicalDeviceSurfacePresentModesKHR(
                             physicalDevice, surface, &presentModeCount, caps.presentModes.data()),
                         "SwapchainCapabilities::Build: "
                         "vkGetPhysicalDeviceSurfacePresentModesKHR failed");
    }

    return caps;
  }

  [[nodiscard]] VkSurfaceFormatKHR ChooseSurfaceFormat() const {
    for (const auto& format : surfaceFormats) {
      if (format.format == VK_FORMAT_R8G8B8A8_SRGB &&
          format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return format;
      }
    }

    if (!surfaceFormats.empty()) {
      return surfaceFormats.front();
    }

    throw std::runtime_error(
        "SwapchainCapabilities::ChooseSurfaceFormat: "
        "no surface formats available");
  }

  [[nodiscard]] VkPresentModeKHR ChoosePresentMode() const {
    for (const auto& mode : presentModes) {
      if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return mode;
      }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
  }

  [[nodiscard]] VkExtent2D ChooseExtent(uint32_t desiredWidth, uint32_t desiredHeight) const {
    if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
      return surfaceCapabilities.currentExtent;
    }

    VkExtent2D extent{desiredWidth, desiredHeight};

    extent.width = std::clamp(extent.width, surfaceCapabilities.minImageExtent.width,
                              surfaceCapabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, surfaceCapabilities.minImageExtent.height,
                               surfaceCapabilities.maxImageExtent.height);

    return extent;
  }

  [[nodiscard]] uint32_t ChooseImageCount() const {
    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;

    if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
      imageCount = surfaceCapabilities.maxImageCount;
    }

    return imageCount;
  }

  [[nodiscard]] VkCompositeAlphaFlagBitsKHR ChooseCompositeAlpha() const {
    constexpr VkCompositeAlphaFlagBitsKHR kPreferredAlphaModes[] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };

    for (VkCompositeAlphaFlagBitsKHR mode : kPreferredAlphaModes) {
      if (surfaceCapabilities.supportedCompositeAlpha & mode) {
        return mode;
      }
    }

    throw std::runtime_error(
        "SwapchainCapabilities::ChooseCompositeAlpha: "
        "no supported composite alpha");
  }

  [[nodiscard]] VkImageUsageFlags ChooseImageUsage() const {
    constexpr VkImageUsageFlags kRequiredUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if ((surfaceCapabilities.supportedUsageFlags & kRequiredUsage) == 0) {
      throw std::runtime_error(
          "SwapchainCapabilities::ChooseImageUsage: "
          "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT not supported");
    }

    return kRequiredUsage;
  }
};

}  // namespace vkcore