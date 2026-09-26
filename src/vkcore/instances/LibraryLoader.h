#pragma once

#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "../common/vulkanFunctions.h"

namespace vkcore {

class LibraryLoader {
 public:
  LibraryLoader();
  ~LibraryLoader();

  [[nodiscard]] const LibraryDispatchTable& dispatchTable() const { return dispatchTable_; }

  [[nodiscard]] const std::vector<std::string>& supportedInstanceExtensions() const {
    return supportedInstanceExtensions_;
  }

  [[nodiscard]] const std::vector<std::string>& supportedInstanceLayers() const {
    return supportedInstanceLayers_;
  }

  [[nodiscard]] bool IsInstanceExtensionSupported(std::string_view extension) const {
    return std::find(supportedInstanceExtensions_.begin(), supportedInstanceExtensions_.end(),
                     extension) != supportedInstanceExtensions_.end();
  }

  [[nodiscard]] bool IsInstanceExtensionSupported(std::string_view extension,
                                                  const std::vector<std::string>& layers) const {
    if (IsInstanceExtensionSupported(extension)) return true;

    for (const auto& layer : layers) {
      const auto it = supportedLayerExtensions_.find(layer);
      if (it == supportedLayerExtensions_.end()) continue;

      const auto& extensions = it->second;
      if (std::find(extensions.begin(), extensions.end(), extension) != extensions.end())
        return true;
    }

    return false;
  }

 private:
#ifdef _WIN32
  HMODULE library_ = nullptr;
#else
  void* library_ = nullptr;
#endif

  LibraryDispatchTable dispatchTable_ = {};
  std::vector<std::string> supportedInstanceExtensions_;
  std::vector<std::string> supportedInstanceLayers_;
  std::unordered_map<std::string, std::vector<std::string>> supportedLayerExtensions_;
};

}  // namespace vkcore