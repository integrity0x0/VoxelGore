#pragma once

#include <optional>
#include <stdexcept>
#include <vector>

#include "../../../../include/glm/glm.hpp"

namespace gfx {

struct ImageRegion {
  glm::ivec2 pos;
  glm::ivec2 size;
  uint32_t arrayLayer;

  ImageRegion() = default;

  ImageRegion(const glm::ivec2& pos, const glm::ivec2& size, uint32_t arrayLayer)
      : pos(pos), size(size), arrayLayer(arrayLayer) {}
};

class ImageRegionAllocator {
 public:
  ImageRegionAllocator(const glm::ivec2& size, uint32_t arrayLayers)
      : size_(size), arrayLayers_(arrayLayers) {
    freeRegions_.resize(arrayLayers);
    for (uint32_t i = 0; i < arrayLayers; ++i) {
      freeRegions_[i] = {ImageRegion(glm::ivec2(0), size, i)};
    }
  }

  std::optional<ImageRegion> reserve(const glm::ivec2& size);

  void free(const ImageRegion& region);

 private:
  glm::ivec2 size_;
  uint32_t arrayLayers_;
  std::vector<std::vector<ImageRegion>> freeRegions_;
};
}  // namespace gfx