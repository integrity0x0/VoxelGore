#include "TextureLoader.h"

namespace gfx {

std::optional<vkcore::SampledTexture> TextureLoader::Load(const vkcore::Device& device,
                                                          vkcore::TransferContext& transferCtxt,
                                                          vkcore::MemoryAllocator& memoryAllocator,
                                                          std::string_view path,
                                                          uint32_t mipLevels) {
  return vkcore::ImportTextureSampled(device, transferCtxt, memoryAllocator, path, mipLevels);
}

std::optional<vkcore::SampledTexture> TextureLoader::LoadCubemap(
    const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
    vkcore::MemoryAllocator& memoryAllocator, const std::array<std::string, 6>& paths,
    uint32_t mipLevels) {
  return vkcore::ImportCubemapSampled(device, transferCtxt, memoryAllocator, paths, mipLevels);
}

}  // namespace gfx