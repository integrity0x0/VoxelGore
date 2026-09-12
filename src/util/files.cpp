#include "files.h"

#include <fstream>

#if defined(__ANDROID__)
#include <android/asset_manager.h>
extern AAssetManager* g_AAssetManager;
#endif

namespace util {

std::vector<std::byte> ReadFileBytes(std::string_view path) {
#if defined(__ANDROID__)
  AAsset* asset = AAssetManager_open(::g_AAssetManager, path.data(), AASSET_MODE_BUFFER);
  if (!asset) {
    return {};
  }

  const off_t length = AAsset_getLength(asset);
  std::vector<std::byte> buffer(static_cast<size_t>(length));

  const int readBytes = AAsset_read(asset, buffer.data(), static_cast<size_t>(length));

  AAsset_close(asset);

  if (readBytes < 0 || static_cast<off_t>(readBytes) != length) {
    return {};
  }

  return buffer;
#else
  std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
  if (!file) {
    return {};
  }

  const std::streamsize size = file.tellg();
  if (size < 0) {
    return {};
  }

  std::vector<std::byte> buffer(static_cast<size_t>(size));

  file.seekg(0, std::ios::beg);
  if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
    return {};
  }

  return buffer;
#endif
}

}  // namespace util