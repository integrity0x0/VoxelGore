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
  [[nodiscard]] T* GetProcAddr(const LibraryLoader& loader, const char* fun) const {
    return reinterpret_cast<T*>(
        loader.dispatchTable().vkGetInstanceProcAddr(instance.get(), fun));
  }

  [[nodiscard]] const std::vector<std::string>& enabledExtensions() const { return enabledExtensions_; }

  [[nodiscard]] const std::vector<std::string> enabledLayers() const { return enabledLayers_; }

  [[nodiscard]] const InstanceDispatchTable& dispatchTable() const { return dispatchTable_; }

  [[nodiscard]] bool IsExtensionEnabled(const std::string& extension) const {
    return std::find(enabledExtensions_.begin(), enabledExtensions_.end(), extension) !=
           enabledExtensions_.end();
  }

  [[nodiscard]] VkInstance handle() const { return instance.get(); }

 private:
  UniqueInstance instance = {};
  std::vector<std::string> enabledExtensions_ = {};
  std::vector<std::string> enabledLayers_ = {};
  uint32_t apiVersion = 0;
  InstanceDispatchTable dispatchTable_ = {};
};
}  // namespace vkcore