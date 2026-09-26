#include "LibraryLoader.h"

namespace vkcore {

LibraryLoader::LibraryLoader() {
#ifdef _WIN32
  library_ = LoadLibraryA("vulkan-1.dll");
  if (!library_)
    throw std::runtime_error(
        "Failed to load vulkan library_ (vulkan-1.dll). "
        "Make sure Vulkan SDK or drivers are installed.");
#else
  library_ = dlopen("libvulkan.so.1", RTLD_LAZY);
  if (!library_) library_ = dlopen("libvulkan.so", RTLD_LAZY);
  if (!library_) library_ = dlopen("libvulkan.dylib", RTLD_LAZY);
  if (!library_) throw std::runtime_error("Failed to load vulkan library");
#endif

  loadBaseLibraryFunctions(library_, dispatchTable_);

  uint32_t extensionsCount = 0;
  VkResult result =
      dispatchTable_.vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

  if (result != VK_SUCCESS)
    throw std::runtime_error("Failed to enumerate instance extension properties (count)");

  std::vector<VkExtensionProperties> extProperties(extensionsCount);
  result = dispatchTable_.vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount,
                                                                 extProperties.data());

  if (result != VK_SUCCESS)
    throw std::runtime_error("Failed to enumerate instance extension properties (data)");

  supportedInstanceExtensions_.reserve(extensionsCount);
  for (const auto& prop : extProperties) {
    supportedInstanceExtensions_.emplace_back(prop.extensionName);
  }

  uint32_t layersCount = 0;
  result = dispatchTable_.vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

  if (result != VK_SUCCESS)
    throw std::runtime_error("Failed to enumerate instance layer properties (count)");

  std::vector<VkLayerProperties> layerProperties(layersCount);
  result = dispatchTable_.vkEnumerateInstanceLayerProperties(&layersCount, layerProperties.data());

  if (result != VK_SUCCESS)
    throw std::runtime_error("Failed to enumerate instance layer properties (data)");

  supportedInstanceLayers_.reserve(layersCount);
  for (const auto& prop : layerProperties) {
    supportedInstanceLayers_.emplace_back(prop.layerName);
  }

  for (const auto& layer : supportedInstanceLayers_) {
    uint32_t layerExtensionsCount = 0;

    result = dispatchTable_.vkEnumerateInstanceExtensionProperties(layer.c_str(),
                                                                   &layerExtensionsCount, nullptr);

    if (result != VK_SUCCESS)
      throw std::runtime_error("Failed to enumerate layer extension properties (count)");

    std::vector<VkExtensionProperties> layerExtensions(layerExtensionsCount);
    result = dispatchTable_.vkEnumerateInstanceExtensionProperties(
        layer.c_str(), &layerExtensionsCount, layerExtensions.data());

    if (result != VK_SUCCESS)
      throw std::runtime_error("Failed to enumerate layer extension properties (data)");

    auto& extensions = supportedLayerExtensions_[layer];
    extensions.reserve(layerExtensionsCount);

    for (const auto& extension : layerExtensions) {
      extensions.emplace_back(extension.extensionName);
    }
  }
}

LibraryLoader::~LibraryLoader() {
#ifdef _WIN32
  if (library_) {
    FreeLibrary(library_);
    library_ = nullptr;
  }
#else
  if (library_) {
    dlclose(library_);
    library_ = nullptr;
  }
#endif
}
}  // namespace vkcore