#include "Instance.h"

#include "../common/vulkanFunctions.h"

namespace vkcore {
Instance::Instance(const LibraryLoader& loader, std::vector<std::string> extensions,
                   std::vector<std::string> layers, const VkApplicationInfo& appInfo,
                   VkInstanceCreateFlags instanceCreateFlags) {
  std::vector<const char*> extensionsCStrings;
  extensionsCStrings.reserve(extensions.size());

  for (const auto& ext : extensions) {
    if (!loader.IsInstanceExtensionSupported(ext, layers)) {
      throw SystemError(VK_ERROR_EXTENSION_NOT_PRESENT,
                        "Failed to create instance: extension " + ext + " is not supported");
    }

    extensionsCStrings.emplace_back(ext.c_str());
  }

  const auto& supportedLayers = loader.supportedInstanceLayers();

  std::vector<const char*> layersCStrings;
  layersCStrings.reserve(layers.size());

  for (const auto& layer : layers) {
    if (std::find(supportedLayers.begin(), supportedLayers.end(), layer) == supportedLayers.end()) {
      throw SystemError(VK_ERROR_LAYER_NOT_PRESENT,
                        "Failed to create instance: layer " + layer + " is not supported");
    }

    layersCStrings.emplace_back(layer.c_str());
  }

  SystemError::Check(loader.dispatchTable().vkEnumerateInstanceVersion(&apiVersion),
                     "Failed to enumerate Vulkan instance version");

  VkInstanceCreateInfo instanceCI = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  instanceCI.flags = instanceCreateFlags;
  instanceCI.pApplicationInfo = &appInfo;
  instanceCI.enabledExtensionCount = static_cast<uint32_t>(extensionsCStrings.size());
  instanceCI.ppEnabledExtensionNames = extensionsCStrings.data();
  instanceCI.enabledLayerCount = static_cast<uint32_t>(layersCStrings.size());
  instanceCI.ppEnabledLayerNames = layersCStrings.data();

  VkInstance inst = VK_NULL_HANDLE;

  SystemError::Check(loader.dispatchTable().vkCreateInstance(&instanceCI, nullptr, &inst),
                     "Failed to create instance");

  PFN_vkDestroyInstance pfnDestroy = reinterpret_cast<PFN_vkDestroyInstance>(
      loader.dispatchTable().vkGetInstanceProcAddr(inst, "vkDestroyInstance"));

  this->instance = UniqueInstance(inst, InstanceDeleter{.func = pfnDestroy});

  enabledExtensions_ = std::move(extensions);
  enabledLayers_ = std::move(layers);

  loadBaseInstanceFunctions(inst, loader.dispatchTable().vkGetInstanceProcAddr, dispatchTable_);

  if (IsExtensionEnabled(VK_KHR_SURFACE_EXTENSION_NAME)) {
    loadSurfaceFunctions(inst, loader.dispatchTable().vkGetInstanceProcAddr, dispatchTable_);
  }

#ifdef VK_USE_PLATFORM_ANDROID_KHR
  if (IsExtensionEnabled(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)) {
    loadAndroidSurfaceFunctions(inst, loader.dispatchTable().vkGetInstanceProcAddr, dispatchTable_);
  }
#endif

  if (IsExtensionEnabled(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
    loadDebugUtilsFunctions(inst, loader.dispatchTable().vkGetInstanceProcAddr, dispatchTable_);
  }
}
}  // namespace vkcore