#pragma once

#include <stdexcept>
#include <vector>

#include "../common/vulkanFunctions.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace vkcore {

class LibraryLoader {
 public:
  LibraryLoader();
  ~LibraryLoader();

  LibraryLoader(const LibraryLoader&) = delete;
  LibraryLoader& operator=(const LibraryLoader&) = delete;
  LibraryLoader(LibraryLoader&&) = delete;
  LibraryLoader& operator=(LibraryLoader&&) = delete;

  const LibraryDispatchTable& getDispatchTable() const;
  const std::vector<std::string>& getSupportedInstanceExtensions() const;
  const std::vector<std::string>& getSupportedInstanceLayers() const;

 private:
#ifdef _WIN32
  HMODULE library = nullptr;
#else
  void* library = nullptr;
#endif
  LibraryDispatchTable dispatchTable{};

  std::vector<std::string> supportedInstanceExtensions;
  std::vector<std::string> supportedInstanceLayers;
};
}  // namespace vkcore