#include "PhysicalDevice.h"

namespace vkcore {

PhysicalDevice::PhysicalDevice(const Instance& instance, VkPhysicalDevice physicalDevice)
    : instance(&instance), physicalDevice(physicalDevice) {
  const auto& dt = instance.getDispatchTable();

  dt.vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  dt.vkGetPhysicalDeviceFeatures(physicalDevice, &features);

  uint32_t extensionsCount = 0u;
  static const char* failedMsg = "Failed to enumerate physical device extension properties";
  SystemError::check(
      dt.vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr),
      failedMsg);

  std::vector<VkExtensionProperties> extProperties(extensionsCount);
  SystemError::check(dt.vkEnumerateDeviceExtensionProperties(
                         physicalDevice, nullptr, &extensionsCount, extProperties.data()),
                     failedMsg);

  supportedExtensions.reserve(extensionsCount);
  for (const auto& ext : extProperties) {
    supportedExtensions.emplace_back(ext.extensionName);
  }

  instance.getDispatchTable().vkGetPhysicalDeviceMemoryProperties(physicalDevice,
                                                                  &memoryProperties);
}

bool PhysicalDevice::supportsExtensions(const std::vector<std::string>& required) const {
  for (const auto& req : required) {
    if (std::find(supportedExtensions.begin(), supportedExtensions.end(), req) ==
        supportedExtensions.end())
      return false;
  }
  return true;
}

bool PhysicalDevice::supportsQueueFamilies(const std::vector<VkQueueFlags>& required) const {
  const auto& dt = instance->getDispatchTable();
  if (!dt.vkGetPhysicalDeviceQueueFamilyProperties) {
    throw std::runtime_error(
        "PhysicalDevice::supportsQueueFamilies: "
        "vkGetPhysicalDeviceQueueFamilyProperties is "
        "missing from dispatch table");
  }

  uint32_t familyCount = 0u;
  dt.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
  std::vector<VkQueueFamilyProperties> families(familyCount);
  dt.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.data());

  for (VkQueueFlags reqFlags : required) {
    bool found = false;
    for (const auto& family : families) {
      if ((family.queueFlags & reqFlags) == reqFlags) {
        found = true;
        break;
      }
    }
    if (!found) return false;
  }
  return true;
}

QueueFamilyIndices PhysicalDevice::getQueueFamilyIndices(VkSurfaceKHR surface) const {
  QueueFamilyIndices result;
  const auto& dt = instance->getDispatchTable();

  if (!dt.vkGetPhysicalDeviceQueueFamilyProperties) {
    throw std::runtime_error(
        "PhysicalDevice::getQueueFamilyIndices: "
        "vkGetPhysicalDeviceQueueFamilyProperties is "
        "missing from dispatch table");
  }

  if (surface != VK_NULL_HANDLE &&
      (!dt.surfaceTable.has_value() || !dt.surfaceTable->vkGetPhysicalDeviceSurfaceSupportKHR)) {
    throw std::runtime_error(
        "PhysicalDevice::getQueueFamilyIndices: surface requested but "
        "surfaceTable / vkGetPhysicalDeviceSurfaceSupportKHR is missing");
  }

  uint32_t familyCount = 0u;
  dt.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
  std::vector<VkQueueFamilyProperties> families(familyCount);
  dt.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.data());

  for (uint32_t i = 0; i < familyCount; ++i) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      result.graphics = i;
    }

    if (surface != VK_NULL_HANDLE) {
      VkBool32 presentSupport = VK_FALSE;
      dt.surfaceTable->vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface,
                                                            &presentSupport);
      if (presentSupport) {
        result.present = i;
      }
    }

    if (result.isComplete()) {
      break;
    }
  }

  return result;
}

int32_t PhysicalDevice::score() const {
  int s = 0;
  if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) s += 1000;
  if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) s += 100;
  return s;
}

std::unique_ptr<PhysicalDevice> pick(const Instance& instance,
                                     const std::vector<std::string>& requiredExtensions,
                                     const std::vector<VkQueueFlags>& requiredFamilies) {
  const auto& dt = instance.getDispatchTable();
  if (!dt.vkEnumeratePhysicalDevices) {
    throw std::runtime_error("pick: vkEnumeratePhysicalDevices is missing from dispatch table");
  }

  uint32_t count = 0u;
  dt.vkEnumeratePhysicalDevices(instance.handle(), &count, nullptr);
  if (count == 0) throw std::runtime_error("No Vulkan physical devices found");

  std::vector<VkPhysicalDevice> devices(count);
  dt.vkEnumeratePhysicalDevices(instance.handle(), &count, devices.data());

  std::unique_ptr<PhysicalDevice> best = nullptr;
  int32_t bestScore = -1;

  for (const auto& dev : devices) {
    auto candidate = std::make_unique<PhysicalDevice>(instance, dev);
    if (!candidate->supportsExtensions(requiredExtensions)) continue;
    if (!candidate->supportsQueueFamilies(requiredFamilies)) continue;

    int32_t s = candidate->score();
    if (s > bestScore) {
      bestScore = s;
      best = std::move(candidate);
    }
  }

  if (best == nullptr) throw std::runtime_error("No suitable physical device found");

  return best;
}

}  // namespace vkcore