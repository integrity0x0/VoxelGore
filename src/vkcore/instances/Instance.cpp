#include "Instance.h"

#include "../common/vulkanFunctions.h"

namespace vkcore {
Instance::Instance(const LibraryLoader& loader, std::vector<std::string> extensions,
                   std::vector<std::string> layers, const VkApplicationInfo& appInfo,
                   VkInstanceCreateFlags instanceCreateFlags) {
  const auto& supportedExtensions = loader.getSupportedInstanceExtensions();

  std::vector<const char*> extensionsCStrings;
  extensionsCStrings.reserve(extensions.size());
  for (const auto& ext : extensions) {
    if (std::find(supportedExtensions.begin(), supportedExtensions.end(), ext) ==
        supportedExtensions.end())
      throw SystemError(VK_ERROR_EXTENSION_NOT_PRESENT,
                        "Failed to create instance: extension " + ext + " is not supported");
    extensionsCStrings.emplace_back(ext.c_str());
  }

  const auto& supportedLayers = loader.getSupportedInstanceLayers();
  std::vector<const char*> layersCStrings;
  layersCStrings.reserve(layers.size());
  for (const auto& layer : layers) {
    if (std::find(supportedLayers.begin(), supportedLayers.end(), layer) == supportedLayers.end())
      throw SystemError(VK_ERROR_EXTENSION_NOT_PRESENT,
                        "Failed to create instance: layer " + layer + " is not supported");
    layersCStrings.emplace_back(layer.c_str());
  }

  loader.getDispatchTable().vkEnumerateInstanceVersion(&apiVersion);

  VkInstanceCreateInfo instanceCI = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  instanceCI.enabledExtensionCount = static_cast<uint32_t>(extensionsCStrings.size());
  instanceCI.ppEnabledExtensionNames = extensionsCStrings.data();
  instanceCI.enabledLayerCount = static_cast<uint32_t>(layersCStrings.size());
  instanceCI.ppEnabledLayerNames = layersCStrings.data();
  instanceCI.pApplicationInfo = &appInfo;
  instanceCI.flags = instanceCreateFlags;

  VkInstance inst = VK_NULL_HANDLE;
  SystemError::Check(loader.getDispatchTable().vkCreateInstance(&instanceCI, nullptr, &inst),
                     "Failed to create instance");

  PFN_vkDestroyInstance pfnDestroy = reinterpret_cast<PFN_vkDestroyInstance>(
      loader.getDispatchTable().vkGetInstanceProcAddr(inst, "vkDestroyInstance"));
  this->instance = UniqueInstance(inst, InstanceDeleter{.func = pfnDestroy});

  enabledExtensions = std::move(extensions);
  enabledLayers = std::move(layers);

  loadBaseInstanceFunctions(inst, loader.getDispatchTable().vkGetInstanceProcAddr, dispatchTable);

  if (isExtensionEnabled(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
    dispatchTable.debugUtilsTable = loadInstanceFunctions<vkcore::DebugUtilsDispatchTable>(
        inst, loader.getDispatchTable().vkGetInstanceProcAddr, debugUtilsFunctions);
  }

  if (isExtensionEnabled(VK_KHR_SURFACE_EXTENSION_NAME)) {
    loadSurfaceFunctions(inst, loader.getDispatchTable().vkGetInstanceProcAddr, dispatchTable);
  }

#ifdef VK_USE_PLATFORM_ANDROID_KHR
  if (isExtensionEnabled(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)) {
    loadAndroidSurfaceFunctions(inst, loader.getDispatchTable().vkGetInstanceProcAddr,
                                dispatchTable);
  }
#endif
  if (isExtensionEnabled(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
    loadDebugUtilsFunctions(inst, loader.getDispatchTable().vkGetInstanceProcAddr, dispatchTable);
  }
}

}  // namespace vkcore