#pragma once

#include <array>
#include <optional>

#include "../../vkcore/resource/SampledTexture.h"

namespace gfx {

class TextureLoader {
 public:
  TextureLoader() = delete;

  [[nodiscard]] static std::optional<vkcore::SampledTexture> Load(
      const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
      vkcore::MemoryAllocator& memoryAllocator, std::string_view path, uint32_t mipLevels = 1u);

  [[nodiscard]] static std::optional<vkcore::SampledTexture> LoadCubemap(
      const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
      vkcore::MemoryAllocator& memoryAllocator, const std::array<std::string, 6>& paths,
      uint32_t mipLevels = 1u);
};

}  // namespace gfx