#include "vulkanFunctions.h"

#include "../instances/LibraryLoader.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include <dlfcn.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

namespace vkcore {

namespace {
static VkResult retVulkan10(uint32_t* pApiVersion) {
  *pApiVersion = VK_API_VERSION_1_0;
  return VK_SUCCESS;
}
}  // namespace

void* loadLibraryFunction(void* library, const std::string& functionName) {
#ifdef _WIN32
  return reinterpret_cast<void*>(
      GetProcAddress(static_cast<HMODULE>(library), functionName.c_str()));
#else
  return dlsym(library, functionName.c_str());
#endif
}

void loadBaseLibraryFunctions(void* module, LibraryDispatchTable& dispatchTable) {
  if (!module)
#ifdef _WIN32
    throw std::runtime_error("vulkan-1.dll is not loaded");
#else
    throw std::runtime_error("libvulkan.so is not loaded");
#endif

  for (auto& func : baseLibraryFunctions) {
    void* address = loadLibraryFunction(module, func.first);
    if (!address) {
      if (strcmp(func.first.c_str(), "vkEnumerateInstanceVersion") == 0)
        address = reinterpret_cast<void*>(retVulkan10);
      else
        throw std::runtime_error("Failed to load vulkan function: '" + std::string(func.first) +
                                 "'");
    }
    void** fieldPtr =
        reinterpret_cast<void**>(reinterpret_cast<char*>(&dispatchTable) + func.second);
    *fieldPtr = address;
  }
}

void loadBaseInstanceFunctions(VkInstance instance, PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                               InstanceDispatchTable& dispatchTable) {
  dispatchTable = loadInstanceFunctions<InstanceDispatchTable>(instance, vkGetInstanceProcAddr,
                                                               baseInstanceFunctions);
}

void loadBaseDeviceFunctions(VkDevice device, PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr,
                             DeviceDispatchTable& dispatchTable) {
  dispatchTable =
      loadDeviceFunctions<DeviceDispatchTable>(device, vkGetDeviceProcAddr, baseDeviceFunctions);
}

void loadSurfaceFunctions(VkInstance instance, PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                          InstanceDispatchTable& dispatchTable) {
  try {
    dispatchTable.surfaceTable = loadInstanceFunctions<SurfaceDispatchTable>(
        instance, vkGetInstanceProcAddr, surfaceFunctions);
  } catch (...) {
    dispatchTable.surfaceTable = std::nullopt;
    throw;
  }
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void loadAndroidSurfaceFunctions(VkInstance instance,
                                 PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                                 InstanceDispatchTable& dispatchTable) {
  try {
    dispatchTable.androidSurfaceTable = loadInstanceFunctions<AndroidSurfaceDispatchTable>(
        instance, vkGetInstanceProcAddr, androidSurfaceFunctions);
  } catch (...) {
    dispatchTable.androidSurfaceTable = std::nullopt;
    throw;
  }
}
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
void loadWin32SurfaceFunctions(VkInstance instance, PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                               InstanceDispatchTable& dispatchTable) {
  try {
    dispatchTable.win32SurfaceTable = loadInstanceFunctions<Win32SurfaceDispatchTable>(
        instance, vkGetInstanceProcAddr, win32SurfaceFunctions);
  } catch (...) {
    dispatchTable.win32SurfaceTable = std::nullopt;
    throw;
  }
}
#endif
void loadDebugUtilsFunctions(VkInstance instance, PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                             InstanceDispatchTable& dispatchTable) {
  try {
    dispatchTable.debugUtilsTable = loadInstanceFunctions<DebugUtilsDispatchTable>(
        instance, vkGetInstanceProcAddr, debugUtilsFunctions);
  } catch (...) {
    dispatchTable.debugUtilsTable = std::nullopt;
  }
}
void loadSwapchainFunctions(VkDevice device, PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr,
                            DeviceDispatchTable& dispatchTable) {
  try {
    dispatchTable.swapchainTable = loadDeviceFunctions<SwapchainDispatchTable>(
        device, vkGetDeviceProcAddr, swapchainFunctions);
  } catch (...) {
    dispatchTable.swapchainTable = std::nullopt;
    throw;
  }
}
}  // namespace vkcore