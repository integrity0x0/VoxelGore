#pragma once

#include <glm/glm.hpp>

#include "../../../vkcore/resource/SampledTexture.h"
#include "../texture/ImageRegionAllocator.h"

namespace gfx {
class GlyphAtlas {
 public:
  static constexpr glm::uvec2 kAtlasResolution = {4096, 4096};
  static constexpr uint32_t kAtlasLayers = 2;

  GlyphAtlas(const vkcore::Device& device, vkcore::MemoryAllocator& memoryAllocator, vkcore::TransferContext& transferCtxt);

  [[nodiscard]] std::optional<ImageRegion> AddGlyph(glm::ivec2 size,
                                                    std::span<const std::byte> pixels);
 private:
  [[nodiscard]] vkcore::SampledTexture CreateTexture(const vkcore::Device& device, vkcore::MemoryAllocator& memoryAllocator);
 private:
  std::reference_wrapper<const vkcore::Device> device_; 
  std::reference_wrapper<vkcore::TransferContext> transferCtxt_;
  vkcore::SampledTexture texture_;
  ImageRegionAllocator regionAllocator_;
};
}  // namespace gfx
