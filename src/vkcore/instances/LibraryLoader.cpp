#include "LibraryLoader.h"

namespace vkcore {

LibraryLoader::LibraryLoader() {
#ifdef _WIN32
  library = LoadLibraryA("vulkan-1.dll");
  if (!library)
    throw std::runtime_error(
        "Failed to load vulkan library (vulkan-1.dll). "
        "Make sure Vulkan SDK or drivers are installed.");
#else
  library = dlopen("libvulkan.so.1", RTLD_LAZY);
  if (!library) library = dlopen("libvulkan.so", RTLD_LAZY);
  if (!library) library = dlopen("libvulkan.dylib", RTLD_LAZY);
  if (!library) throw std::runtime_error("Failed to load vulkan library");
#endif

  loadBaseLibraryFunctions(library, dispatchTable);

  uint32_t extensionsCount = 0;
  VkResult result =
      dispatchTable.vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
  if (result != VK_SUCCESS)
    throw std::runtime_error("Failed to enumerate instance extension properties (count)");

  std::vector<VkExtensionProperties> extProperties(extensionsCount);
  result = dispatchTable.vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount,
                                                                extProperties.data());
  if (result != VK_SUCCESS)
    throw std::runtime_error("Failed to enumerate instance extension properties (data)");

  supportedInstanceExtensions.reserve(extensionsCount);
  for (const auto& prop : extProperties) {
    supportedInstanceExtensions.emplace_back(prop.extensionName);
  }

  uint32_t layersCount = 0;
  result = dispatchTable.vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
  if (result != VK_SUCCESS)
    throw std::runtime_error("Failed to enumerate instance layer properties (count)");

  std::vector<VkLayerProperties> layerProperties(layersCount);
  result = dispatchTable.vkEnumerateInstanceLayerProperties(&layersCount, layerProperties.data());
  if (result != VK_SUCCESS)
    throw std::runtime_error("Failed to enumerate instance layer properties (data)");

  supportedInstanceLayers.reserve(layersCount);
  for (const auto& prop : layerProperties) {
    supportedInstanceLayers.emplace_back(prop.layerName);
  }
}

LibraryLoader::~LibraryLoader() {
#ifdef _WIN32
  if (library) {
    FreeLibrary(library);
    library = nullptr;
  }
#else
  if (library) {
    dlclose(library);
    library = nullptr;
  }
#endif
}

const LibraryDispatchTable& LibraryLoader::getDispatchTable() const { return dispatchTable; }

const std::vector<std::string>& LibraryLoader::getSupportedInstanceExtensions() const {
  return supportedInstanceExtensions;
}

const std::vector<std::string>& LibraryLoader::getSupportedInstanceLayers() const {
  return supportedInstanceLayers;
}
}  // namespace vkcore