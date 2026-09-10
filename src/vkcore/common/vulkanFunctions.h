#pragma once
#include <assert.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#ifdef _WIN32
#else
#include <dlfcn.h>
#endif

namespace vkcore {
// =========================================================================
// Library dispatch table
// =========================================================================

struct LibraryDispatchTable {
  PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
  PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
  PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
  PFN_vkCreateInstance vkCreateInstance;
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
};

static std::unordered_map<std::string, size_t> baseLibraryFunctions = {
    {"vkEnumerateInstanceVersion", offsetof(LibraryDispatchTable, vkEnumerateInstanceVersion)},
    {"vkEnumerateInstanceExtensionProperties",
     offsetof(LibraryDispatchTable, vkEnumerateInstanceExtensionProperties)},
    {"vkEnumerateInstanceLayerProperties",
     offsetof(LibraryDispatchTable, vkEnumerateInstanceLayerProperties)},
    {"vkCreateInstance", offsetof(LibraryDispatchTable, vkCreateInstance)},
    {"vkGetInstanceProcAddr", offsetof(LibraryDispatchTable, vkGetInstanceProcAddr)},
};

// =========================================================================
// Instance extension tables
// =========================================================================

struct DebugUtilsDispatchTable {
  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
  PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;
};

static std::unordered_map<std::string, size_t> debugUtilsFunctions = {
    {"vkCreateDebugUtilsMessengerEXT",
     offsetof(DebugUtilsDispatchTable, vkCreateDebugUtilsMessengerEXT)},
    {"vkDestroyDebugUtilsMessengerEXT",
     offsetof(DebugUtilsDispatchTable, vkDestroyDebugUtilsMessengerEXT)},
};

struct SurfaceDispatchTable {
  PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
};

static std::unordered_map<std::string, size_t> surfaceFunctions = {
    {"vkDestroySurfaceKHR", offsetof(SurfaceDispatchTable, vkDestroySurfaceKHR)},
    {"vkGetPhysicalDeviceSurfaceSupportKHR",
     offsetof(SurfaceDispatchTable, vkGetPhysicalDeviceSurfaceSupportKHR)},
    {"vkGetPhysicalDeviceSurfaceCapabilitiesKHR",
     offsetof(SurfaceDispatchTable, vkGetPhysicalDeviceSurfaceCapabilitiesKHR)},
    {"vkGetPhysicalDeviceSurfaceFormatsKHR",
     offsetof(SurfaceDispatchTable, vkGetPhysicalDeviceSurfaceFormatsKHR)},
    {"vkGetPhysicalDeviceSurfacePresentModesKHR",
     offsetof(SurfaceDispatchTable, vkGetPhysicalDeviceSurfacePresentModesKHR)},
};

#ifdef VK_USE_PLATFORM_ANDROID_KHR
struct AndroidSurfaceDispatchTable {
  PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR = nullptr;
};

static std::unordered_map<std::string, size_t> androidSurfaceFunctions = {
    {"vkCreateAndroidSurfaceKHR", offsetof(AndroidSurfaceDispatchTable, vkCreateAndroidSurfaceKHR)},
};
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct Win32SurfaceDispatchTable {
  PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = nullptr;
};

static std::unordered_map<std::string, size_t> win32SurfaceFunctions = {
    {"vkCreateWin32SurfaceKHR", offsetof(Win32SurfaceDispatchTable, vkCreateWin32SurfaceKHR)},
};
#endif

// =========================================================================
// Instance dispatch table
// =========================================================================

struct InstanceDispatchTable {
  // instance
  PFN_vkDestroyInstance vkDestroyInstance;
  // physical device
  PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
  PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
  PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
  PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
  PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
  // queues
  PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
  // logical device
  PFN_vkCreateDevice vkCreateDevice;
  PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
  // extension tables
  std::optional<DebugUtilsDispatchTable> debugUtilsTable = std::nullopt;
  std::optional<SurfaceDispatchTable> surfaceTable = std::nullopt;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
  std::optional<AndroidSurfaceDispatchTable> androidSurfaceTable = std::nullopt;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
  std::optional<Win32SurfaceDispatchTable> win32SurfaceTable = std::nullopt;
#endif
};

static std::unordered_map<std::string, size_t> baseInstanceFunctions = {
    {"vkDestroyInstance", offsetof(InstanceDispatchTable, vkDestroyInstance)},
    {"vkEnumeratePhysicalDevices", offsetof(InstanceDispatchTable, vkEnumeratePhysicalDevices)},
    {"vkGetPhysicalDeviceProperties",
     offsetof(InstanceDispatchTable, vkGetPhysicalDeviceProperties)},
    {"vkGetPhysicalDeviceFeatures", offsetof(InstanceDispatchTable, vkGetPhysicalDeviceFeatures)},
    {"vkGetPhysicalDeviceMemoryProperties",
     offsetof(InstanceDispatchTable, vkGetPhysicalDeviceMemoryProperties)},
    {"vkEnumerateDeviceExtensionProperties",
     offsetof(InstanceDispatchTable, vkEnumerateDeviceExtensionProperties)},
    {"vkGetPhysicalDeviceQueueFamilyProperties",
     offsetof(InstanceDispatchTable, vkGetPhysicalDeviceQueueFamilyProperties)},
    {"vkCreateDevice", offsetof(InstanceDispatchTable, vkCreateDevice)},
    {"vkGetDeviceProcAddr", offsetof(InstanceDispatchTable, vkGetDeviceProcAddr)},
};

// =========================================================================
// Device dispatch table
// =========================================================================
struct SwapchainDispatchTable {
  PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
  PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
  PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
  PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
  PFN_vkQueuePresentKHR vkQueuePresentKHR;
};

const std::unordered_map<std::string, size_t> swapchainFunctions = {
    {"vkCreateSwapchainKHR", offsetof(SwapchainDispatchTable, vkCreateSwapchainKHR)},
    {"vkDestroySwapchainKHR", offsetof(SwapchainDispatchTable, vkDestroySwapchainKHR)},
    {"vkGetSwapchainImagesKHR", offsetof(SwapchainDispatchTable, vkGetSwapchainImagesKHR)},
    {"vkAcquireNextImageKHR", offsetof(SwapchainDispatchTable, vkAcquireNextImageKHR)},
    {"vkQueuePresentKHR", offsetof(SwapchainDispatchTable, vkQueuePresentKHR)},
};

struct DeviceDispatchTable {
  // device
  PFN_vkDestroyDevice vkDestroyDevice;
  PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
  // queues
  PFN_vkGetDeviceQueue vkGetDeviceQueue;
  PFN_vkQueueSubmit vkQueueSubmit;
  PFN_vkQueueWaitIdle vkQueueWaitIdle;
  PFN_vkQueuePresentKHR vkQueuePresentKHR;
  // swapchain
  PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
  PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
  PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
  PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
  // command pool/buffer
  PFN_vkCreateCommandPool vkCreateCommandPool;
  PFN_vkDestroyCommandPool vkDestroyCommandPool;
  PFN_vkResetCommandPool vkResetCommandPool;
  PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
  PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
  PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
  PFN_vkEndCommandBuffer vkEndCommandBuffer;
  PFN_vkResetCommandBuffer vkResetCommandBuffer;
  // sync
  PFN_vkCreateFence vkCreateFence;
  PFN_vkDestroyFence vkDestroyFence;
  PFN_vkWaitForFences vkWaitForFences;
  PFN_vkGetFenceStatus vkGetFenceStatus;
  PFN_vkResetFences vkResetFences;
  PFN_vkCreateSemaphore vkCreateSemaphore;
  PFN_vkDestroySemaphore vkDestroySemaphore;
  // render pass
  PFN_vkCreateRenderPass vkCreateRenderPass;
  PFN_vkDestroyRenderPass vkDestroyRenderPass;
  PFN_vkCreateFramebuffer vkCreateFramebuffer;
  PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
  // image/view
  PFN_vkCreateImageView vkCreateImageView;
  PFN_vkDestroyImageView vkDestroyImageView;
  PFN_vkCreateImage vkCreateImage;
  PFN_vkDestroyImage vkDestroyImage;
  // pipeline
  PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
  PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
  PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
  PFN_vkDestroyPipeline vkDestroyPipeline;
  PFN_vkCreateShaderModule vkCreateShaderModule;
  PFN_vkDestroyShaderModule vkDestroyShaderModule;
  // descriptor
  PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
  PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
  PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
  PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
  PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
  PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
  PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
  // memory/buffer
  PFN_vkAllocateMemory vkAllocateMemory;
  PFN_vkFreeMemory vkFreeMemory;
  PFN_vkMapMemory vkMapMemory;
  PFN_vkUnmapMemory vkUnmapMemory;
  PFN_vkCreateBuffer vkCreateBuffer;
  PFN_vkDestroyBuffer vkDestroyBuffer;
  PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
  PFN_vkBindBufferMemory vkBindBufferMemory;
  PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
  PFN_vkBindImageMemory vkBindImageMemory;
  // draw commands
  PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
  PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
  PFN_vkCmdBindPipeline vkCmdBindPipeline;
  PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
  PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
  PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
  PFN_vkCmdDraw vkCmdDraw;
  PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
  PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
  PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
  PFN_vkCmdBlitImage vkCmdBlitImage;
  PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
  PFN_vkCmdSetViewport vkCmdSetViewport;
  PFN_vkCmdSetScissor vkCmdSetScissor;
  PFN_vkCmdPushConstants vkCmdPushConstants;
  PFN_vkCreateSampler vkCreateSampler;
  PFN_vkDestroySampler vkDestroySampler;

  std::optional<SwapchainDispatchTable> swapchainTable = std::nullopt;
};

static std::unordered_map<std::string, size_t> baseDeviceFunctions = {
    {"vkDestroyDevice", offsetof(DeviceDispatchTable, vkDestroyDevice)},
    {"vkDeviceWaitIdle", offsetof(DeviceDispatchTable, vkDeviceWaitIdle)},
    {"vkGetDeviceQueue", offsetof(DeviceDispatchTable, vkGetDeviceQueue)},
    {"vkQueueSubmit", offsetof(DeviceDispatchTable, vkQueueSubmit)},
    {"vkQueueWaitIdle", offsetof(DeviceDispatchTable, vkQueueWaitIdle)},
    {"vkQueuePresentKHR", offsetof(DeviceDispatchTable, vkQueuePresentKHR)},
    {"vkCreateSwapchainKHR", offsetof(DeviceDispatchTable, vkCreateSwapchainKHR)},
    {"vkDestroySwapchainKHR", offsetof(DeviceDispatchTable, vkDestroySwapchainKHR)},
    {"vkGetSwapchainImagesKHR", offsetof(DeviceDispatchTable, vkGetSwapchainImagesKHR)},
    {"vkAcquireNextImageKHR", offsetof(DeviceDispatchTable, vkAcquireNextImageKHR)},
    {"vkCreateCommandPool", offsetof(DeviceDispatchTable, vkCreateCommandPool)},
    {"vkDestroyCommandPool", offsetof(DeviceDispatchTable, vkDestroyCommandPool)},
    {"vkResetCommandPool", offsetof(DeviceDispatchTable, vkResetCommandPool)},
    {"vkAllocateCommandBuffers", offsetof(DeviceDispatchTable, vkAllocateCommandBuffers)},
    {"vkFreeCommandBuffers", offsetof(DeviceDispatchTable, vkFreeCommandBuffers)},
    {"vkBeginCommandBuffer", offsetof(DeviceDispatchTable, vkBeginCommandBuffer)},
    {"vkEndCommandBuffer", offsetof(DeviceDispatchTable, vkEndCommandBuffer)},
    {"vkResetCommandBuffer", offsetof(DeviceDispatchTable, vkResetCommandBuffer)},
    {"vkCreateFence", offsetof(DeviceDispatchTable, vkCreateFence)},
    {"vkDestroyFence", offsetof(DeviceDispatchTable, vkDestroyFence)},
    {"vkWaitForFences", offsetof(DeviceDispatchTable, vkWaitForFences)},
    {"vkGetFenceStatus", offsetof(DeviceDispatchTable, vkGetFenceStatus)},
    {"vkResetFences", offsetof(DeviceDispatchTable, vkResetFences)},
    {"vkCreateSemaphore", offsetof(DeviceDispatchTable, vkCreateSemaphore)},
    {"vkDestroySemaphore", offsetof(DeviceDispatchTable, vkDestroySemaphore)},
    {"vkCreateRenderPass", offsetof(DeviceDispatchTable, vkCreateRenderPass)},
    {"vkDestroyRenderPass", offsetof(DeviceDispatchTable, vkDestroyRenderPass)},
    {"vkCreateFramebuffer", offsetof(DeviceDispatchTable, vkCreateFramebuffer)},
    {"vkDestroyFramebuffer", offsetof(DeviceDispatchTable, vkDestroyFramebuffer)},
    {"vkCreateImageView", offsetof(DeviceDispatchTable, vkCreateImageView)},
    {"vkDestroyImageView", offsetof(DeviceDispatchTable, vkDestroyImageView)},
    {"vkCreateImage", offsetof(DeviceDispatchTable, vkCreateImage)},
    {"vkDestroyImage", offsetof(DeviceDispatchTable, vkDestroyImage)},
    {"vkCreatePipelineLayout", offsetof(DeviceDispatchTable, vkCreatePipelineLayout)},
    {"vkDestroyPipelineLayout", offsetof(DeviceDispatchTable, vkDestroyPipelineLayout)},
    {"vkCreateGraphicsPipelines", offsetof(DeviceDispatchTable, vkCreateGraphicsPipelines)},
    {"vkDestroyPipeline", offsetof(DeviceDispatchTable, vkDestroyPipeline)},
    {"vkCreateShaderModule", offsetof(DeviceDispatchTable, vkCreateShaderModule)},
    {"vkDestroyShaderModule", offsetof(DeviceDispatchTable, vkDestroyShaderModule)},
    {"vkCreateDescriptorSetLayout", offsetof(DeviceDispatchTable, vkCreateDescriptorSetLayout)},
    {"vkDestroyDescriptorSetLayout", offsetof(DeviceDispatchTable, vkDestroyDescriptorSetLayout)},
    {"vkCreateDescriptorPool", offsetof(DeviceDispatchTable, vkCreateDescriptorPool)},
    {"vkDestroyDescriptorPool", offsetof(DeviceDispatchTable, vkDestroyDescriptorPool)},
    {"vkAllocateDescriptorSets", offsetof(DeviceDispatchTable, vkAllocateDescriptorSets)},
    {"vkUpdateDescriptorSets", offsetof(DeviceDispatchTable, vkUpdateDescriptorSets)},
    {"vkFreeDescriptorSets", offsetof(DeviceDispatchTable, vkFreeDescriptorSets)},
    {"vkAllocateMemory", offsetof(DeviceDispatchTable, vkAllocateMemory)},
    {"vkFreeMemory", offsetof(DeviceDispatchTable, vkFreeMemory)},
    {"vkMapMemory", offsetof(DeviceDispatchTable, vkMapMemory)},
    {"vkUnmapMemory", offsetof(DeviceDispatchTable, vkUnmapMemory)},
    {"vkCreateBuffer", offsetof(DeviceDispatchTable, vkCreateBuffer)},
    {"vkDestroyBuffer", offsetof(DeviceDispatchTable, vkDestroyBuffer)},
    {"vkGetBufferMemoryRequirements", offsetof(DeviceDispatchTable, vkGetBufferMemoryRequirements)},
    {"vkBindBufferMemory", offsetof(DeviceDispatchTable, vkBindBufferMemory)},
    {"vkGetImageMemoryRequirements", offsetof(DeviceDispatchTable, vkGetImageMemoryRequirements)},
    {"vkBindImageMemory", offsetof(DeviceDispatchTable, vkBindImageMemory)},
    {"vkCmdBeginRenderPass", offsetof(DeviceDispatchTable, vkCmdBeginRenderPass)},
    {"vkCmdEndRenderPass", offsetof(DeviceDispatchTable, vkCmdEndRenderPass)},
    {"vkCmdBindPipeline", offsetof(DeviceDispatchTable, vkCmdBindPipeline)},
    {"vkCmdBindVertexBuffers", offsetof(DeviceDispatchTable, vkCmdBindVertexBuffers)},
    {"vkCmdBindIndexBuffer", offsetof(DeviceDispatchTable, vkCmdBindIndexBuffer)},
    {"vkCmdBindDescriptorSets", offsetof(DeviceDispatchTable, vkCmdBindDescriptorSets)},
    {"vkCmdDraw", offsetof(DeviceDispatchTable, vkCmdDraw)},
    {"vkCmdDrawIndexed", offsetof(DeviceDispatchTable, vkCmdDrawIndexed)},
    {"vkCmdCopyBuffer", offsetof(DeviceDispatchTable, vkCmdCopyBuffer)},
    {"vkCmdCopyBufferToImage", offsetof(DeviceDispatchTable, vkCmdCopyBufferToImage)},
    {"vkCmdBlitImage", offsetof(DeviceDispatchTable, vkCmdBlitImage)},
    {"vkCmdPipelineBarrier", offsetof(DeviceDispatchTable, vkCmdPipelineBarrier)},
    {"vkCmdSetViewport", offsetof(DeviceDispatchTable, vkCmdSetViewport)},
    {"vkCmdSetScissor", offsetof(DeviceDispatchTable, vkCmdSetScissor)},
    {"vkCmdPushConstants", offsetof(DeviceDispatchTable, vkCmdPushConstants)},
    {"vkCreateSampler", offsetof(DeviceDispatchTable, vkCreateSampler)},
    {"vkDestroySampler", offsetof(DeviceDispatchTable, vkDestroySampler)}};

// =========================================================================
// Load functions
// =========================================================================

extern void* loadLibraryFunction(void* library, const std::string& functionName);
extern void loadBaseLibraryFunctions(void* library, LibraryDispatchTable& dispatchTable);
extern void loadBaseInstanceFunctions(VkInstance instance,
                                      PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                                      InstanceDispatchTable& dispatchTable);
extern void loadSurfaceFunctions(VkInstance instance,
                                 PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                                 InstanceDispatchTable& dispatchTable);

#ifdef VK_USE_PLATFORM_ANDROID_KHR
extern void loadAndroidSurfaceFunctions(VkInstance instance,
                                        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                                        InstanceDispatchTable& dispatchTable);
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
extern void loadWin32SurfaceFunctions(VkInstance instance,
                                      PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                                      InstanceDispatchTable& dispatchTable);
#endif
extern void loadDebugUtilsFunctions(VkInstance instance,
                                    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                                    InstanceDispatchTable& dispatchTable);

extern void loadBaseDeviceFunctions(VkDevice device, PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr,
                                    DeviceDispatchTable& dispatchTable);
extern void loadSwapchainFunctions(VkDevice device, PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr,
                                   DeviceDispatchTable& dispatchTable);

template <typename T>
static T loadInstanceFunctions(VkInstance instance, PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                               const std::unordered_map<std::string, size_t>& dispatchOffsetTable) {
  T result = T();
  for (const auto& func : dispatchOffsetTable) {
    void* funPtr = reinterpret_cast<void*>(vkGetInstanceProcAddr(instance, func.first.c_str()));
    if (!funPtr) throw std::runtime_error("Failed to load instance function: '" + func.first + "'");
    assert(func.second + sizeof(void*) <= sizeof(T));
    void** fieldPtr = reinterpret_cast<void**>(reinterpret_cast<char*>(&result) + func.second);
    *fieldPtr = funPtr;
  }
  return result;
}

template <typename T>
static T loadDeviceFunctions(VkDevice device, PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr,
                             const std::unordered_map<std::string, size_t>& dispatchOffsetTable) {
  T result = T();
  for (const auto& func : dispatchOffsetTable) {
    void* funPtr = reinterpret_cast<void*>(vkGetDeviceProcAddr(device, func.first.c_str()));
    if (!funPtr) throw std::runtime_error("Failed to load device function: '" + func.first + "'");
    assert(func.second + sizeof(void*) <= sizeof(T));
    void** fieldPtr = reinterpret_cast<void**>(reinterpret_cast<char*>(&result) + func.second);
    *fieldPtr = funPtr;
  }
  return result;
}
}  // namespace vkcore