#include "DeviceCreator.h"

#include <unordered_map>

#include "../common/SystemError.h"
#include "../common/vulkanFunctions.h"

namespace vkcore {

Device DeviceCreator::createDevice() {
  std::unordered_map<uint32_t, std::vector<float>> prioritiesByFamily;
  for (const auto& [familyIndex, priority] : queueRequests_) {
    prioritiesByFamily[familyIndex].push_back(priority);
  }

  std::vector<VkDeviceQueueCreateInfo> queueCIs;
  queueCIs.reserve(prioritiesByFamily.size());
  for (auto& [family, priorities] : prioritiesByFamily) {
    VkDeviceQueueCreateInfo ci = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    ci.queueFamilyIndex = family;
    ci.queueCount = static_cast<uint32_t>(priorities.size());
    ci.pQueuePriorities = priorities.data();
    queueCIs.push_back(ci);
  }

  std::vector<const char*> extensionsCStrings;
  extensionsCStrings.reserve(extensions_.size());
  for (const auto& ext : extensions_) {
    extensionsCStrings.push_back(ext.c_str());
  }

  std::vector<const char*> layersCStrings;
  layersCStrings.reserve(validationLayers_.size());
  for (const auto& layer : validationLayers_) {
    layersCStrings.push_back(layer.c_str());
  }

  VkDeviceCreateInfo deviceCI = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  deviceCI.queueCreateInfoCount = static_cast<uint32_t>(queueCIs.size());
  deviceCI.pQueueCreateInfos = queueCIs.data();
  deviceCI.enabledExtensionCount = static_cast<uint32_t>(extensionsCStrings.size());
  deviceCI.ppEnabledExtensionNames = extensionsCStrings.data();
  deviceCI.enabledLayerCount = static_cast<uint32_t>(layersCStrings.size());
  deviceCI.ppEnabledLayerNames = layersCStrings.data();
  deviceCI.pEnabledFeatures = &enabledFeatures_;

  VkDevice devHandle = VK_NULL_HANDLE;
  SystemError::Check(instance_->dispatchTable().vkCreateDevice(physicalDevice_.handle(),
                                                                  &deviceCI, nullptr, &devHandle),
                     "Failed to create logical device");

  PFN_vkDestroyDevice pfnDestroy = reinterpret_cast<PFN_vkDestroyDevice>(
      instance_->dispatchTable().vkGetDeviceProcAddr(devHandle, "vkDestroyDevice"));
  UniqueDevice uniqueDevice(devHandle, DeviceDeleter{.func = pfnDestroy});

  DeviceDispatchTable dispatchTable{};
  loadBaseDeviceFunctions(devHandle, instance_->dispatchTable().vkGetDeviceProcAddr,
                          dispatchTable);

  bool swapchainEnabled = std::find(extensions_.begin(), extensions_.end(),
                                    VK_KHR_SWAPCHAIN_EXTENSION_NAME) != extensions_.end();

  if (swapchainEnabled) {
    loadSwapchainFunctions(devHandle, instance_->dispatchTable().vkGetDeviceProcAddr,
                           dispatchTable);
  }

  uint32_t familyCount = 0;
  instance_->dispatchTable().vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_.handle(),
                                                                         &familyCount, nullptr);
  std::vector<VkQueueFamilyProperties> familyProps(familyCount);
  instance_->dispatchTable().vkGetPhysicalDeviceQueueFamilyProperties(
      physicalDevice_.handle(), &familyCount, familyProps.data());

  std::vector<DeviceQueue> deviceQueues;
  deviceQueues.reserve(queueRequests_.size());

  std::unordered_map<uint32_t, uint32_t> nextIndexInFamily;
  for (const auto& [familyIndex, priority] : queueRequests_) {
    uint32_t indexInFamily = nextIndexInFamily[familyIndex]++;

    VkQueue q = VK_NULL_HANDLE;
    dispatchTable.vkGetDeviceQueue(devHandle, familyIndex, indexInFamily, &q);

    VkQueueFlags flags =
        (familyIndex < familyProps.size()) ? familyProps[familyIndex].queueFlags : 0;

    deviceQueues.emplace_back(dispatchTable, q, familyIndex, flags);
  }

  std::vector<std::string> enabledExtCopy = extensions_;

  return Device(std::move(uniqueDevice), dispatchTable, std::move(enabledExtCopy), enabledFeatures_,
                std::move(deviceQueues), physicalDevice_);
}

}  // namespace vkcore