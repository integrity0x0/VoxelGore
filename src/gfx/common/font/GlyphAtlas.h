#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include "../../../vkcore/resource/SampledTexture.h"
#include "../../../vkcore/resource/TransferContext.h"
#include "../texture/ImageRegionAllocator.h"
#include "../texture/MaterialManager.h"
#include "Glyph.h"

namespace gfx {
class GlyphAtlas {
 public:
  static constexpr glm::ivec2 kAtlasResolution = {4096, 4096};
  static constexpr uint32_t kMaxPages = 4;
  static constexpr int32_t kGlyphPadding = 1;

  GlyphAtlas(const vkcore::Device& device, vkcore::MemoryAllocator& memoryAllocator,
             vkcore::TransferContext& transferCtxt, MaterialManager& materialManager);

  [[nodiscard]] std::optional<GlyphSlot> AddGlyph(glm::ivec2 size,
                                                  std::span<const std::byte> pixels);

 private:
  struct Page {
    vkcore::SampledTexture texture;
    ImageRegionAllocator allocator;
    MaterialId materialId = kInvalidMaterialId;

    explicit Page(vkcore::SampledTexture&& texture)
        : texture(std::move(texture)), allocator(kAtlasResolution, 1) {}
  };

  [[nodiscard]] vkcore::SampledTexture CreateTexture() const;
  [[nodiscard]] Page* AddPage();
  void Upload(Page& page, const ImageRegion& region, std::span<const std::byte> pixels);

 private:
  std::reference_wrapper<const vkcore::Device> device_;
  std::reference_wrapper<vkcore::MemoryAllocator> memoryAllocator_;
  std::reference_wrapper<vkcore::TransferContext> transferCtxt_;
  std::reference_wrapper<MaterialManager> materialManager_;
  std::vector<std::unique_ptr<Page>> pages_; 
  std::vector<std::byte> paddedPixels_;
};
}  // namespace gfx