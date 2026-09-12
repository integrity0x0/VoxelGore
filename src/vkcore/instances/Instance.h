#pragma once

#include <algorithm>

#include "../common/SystemError.h"
#include "../common/UniqueHandles.h"
#include "LibraryLoader.h"

namespace vkcore {
class Instance {
 public:
  Instance(const LibraryLoader& loader, std::vector<std::string> extensions,
           std::vector<std::string> layers, const VkApplicationInfo& appInfo,
           VkInstanceCreateFlags instanceCreateFlags = 0);
  ~Instance() = default;

  template <typename T>
  inline T* getProcAddr(const LibraryLoader& loader, const char* fun) const {
    return reinterpret_cast<T*>(
        loader.getDispatchTable().vkGetInstanceProcAddr(instance.get(), fun));
  }

  inline const std::vector<std::string>& getEnabledExtensions() const { return enabledExtensions; }

  inline const std::vector<std::string> getEnabledLayers() const { return enabledLayers; }

  inline const InstanceDispatchTable& getDispatchTable() const { return dispatchTable; }

  inline bool isExtensionEnabled(const std::string& extension) const {
    return std::find(enabledExtensions.begin(), enabledExtensions.end(), extension) !=
           enabledExtensions.end();
  }

  inline VkInstance handle() const { return instance.get(); }

 private:
  UniqueInstance instance = {};
  std::vector<std::string> enabledExtensions = {};
  std::vector<std::string> enabledLayers = {};
  uint32_t apiVersion = 0;
  InstanceDispatchTable dispatchTable = {};
};
}  // namespace vkcore