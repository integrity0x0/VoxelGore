#include "TextureLoader.h"

#include <stb_image.h>

#include <memory>
#include <stdexcept>

#include "../../../util/files.h"

namespace gfx {

namespace {

struct StbiDeleter {
  void operator()(stbi_uc* p) const { stbi_image_free(p); }
};

struct LoadedImage {
  int width = 0;
  int height = 0;
  std::unique_ptr<stbi_uc, StbiDeleter> pixels;
};

LoadedImage LoadImagePixels(std::string_view path) {
  auto bytes = util::ReadFileBytes(path);
  if (bytes.empty()) {
    throw std::runtime_error("failed to read file: " + std::string(path));
  }

  int width = 0, height = 0, nrChannels = 0;
  stbi_uc* raw = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(bytes.data()),
                                       static_cast<int>(bytes.size()), &width, &height, &nrChannels,
                                       STBI_rgb_alpha);
  if (!raw) {
    throw std::runtime_error("failed to decode image: " + std::string(path));
  }

  return {width, height, std::unique_ptr<stbi_uc, StbiDeleter>(raw)};
}

}  // namespace

std::optional<vkcore::SampledTexture> TextureLoader::Load(const vkcore::Device& device,
                                                          vkcore::TransferContext& transferCtxt,
                                                          vkcore::MemoryAllocator& memoryAllocator,
                                                          std::string_view path,
                                                          uint32_t mipLevels) {
  try {
    const LoadedImage loaded = LoadImagePixels(path);
    const size_t size = static_cast<size_t>(loaded.width) * static_cast<size_t>(loaded.height) * 4u;

    return vkcore::MakeTextureSampled(
        device, transferCtxt, memoryAllocator,
        {reinterpret_cast<const std::byte*>(loaded.pixels.get()), size},
        static_cast<uint32_t>(loaded.width), static_cast<uint32_t>(loaded.height), mipLevels);
  } catch (...) {
    return std::nullopt;
  }
}

std::optional<vkcore::SampledTexture> TextureLoader::LoadCubemap(
    const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
    vkcore::MemoryAllocator& memoryAllocator, const std::array<std::string, 6>& paths,
    uint32_t mipLevels) {
  try {
    std::array<LoadedImage, 6> loaded;
    for (uint32_t i = 0; i < 6u; ++i) {
      loaded[i] = LoadImagePixels(paths[i]);
    }

    const int width = loaded[0].width;
    const int height = loaded[0].height;

    for (uint32_t i = 1; i < 6u; ++i) {
      if (loaded[i].width != width || loaded[i].height != height) {
        throw std::runtime_error("cubemap faces have different dimensions");
      }
    }
    if (width != height) {
      throw std::runtime_error("cubemap faces must be square");
    }

    size_t faceSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;

    std::array<std::span<const std::byte>, 6> faces;
    for (uint32_t i = 0; i < 6u; ++i) {
      faces[i] = {reinterpret_cast<const std::byte*>(loaded[i].pixels.get()), faceSize};
    }

    return vkcore::MakeCubemapSampled(device, transferCtxt, memoryAllocator, faces,
                                      static_cast<uint32_t>(width), mipLevels);
  } catch (...) {
    return std::nullopt;
  }
}

}  // namespace gfx