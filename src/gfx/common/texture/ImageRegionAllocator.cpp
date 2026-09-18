#include "ImageRegionAllocator.h"

namespace gfx {

std::optional<ImageRegion> ImageRegionAllocator::reserve(const glm::ivec2& size) {
  for (uint32_t layer = 0; layer < arrayLayers_; ++layer) {
    auto& regionList = freeRegions_[layer];
    auto it =
        std::find_if(regionList.begin(), regionList.end(), [size](const ImageRegion& freeRegion) {
          return freeRegion.size.x >= size.x && freeRegion.size.y >= size.y;
        });

    if (it != regionList.end()) {
      ImageRegion region = *it;
      regionList.erase(it);

      glm::ivec2 bottomPos(region.pos.x, region.pos.y + size.y);
      glm::ivec2 bottomSize(region.size.x, region.size.y - size.y);

      glm::ivec2 rightPos(region.pos.x + size.x, region.pos.y);
      glm::ivec2 rightSize(region.size.x - size.x, size.y);

      if (bottomSize.x > 0 && bottomSize.y > 0) {
        regionList.emplace_back(bottomPos, bottomSize, layer);
      }
      if (rightSize.x > 0 && rightSize.y > 0) {
        regionList.emplace_back(rightPos, rightSize, layer);
      }

      return ImageRegion(region.pos, size, layer);
    }
  }
  return std::nullopt;
}

void ImageRegionAllocator::free(const ImageRegion& region) {
  throw std::runtime_error("похуй потом");
}

}  // namespace gfx