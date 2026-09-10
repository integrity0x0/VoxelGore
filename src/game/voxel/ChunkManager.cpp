#include "ChunkManager.h"

namespace gm {

ChunkManager::ChunkManager(uint32_t width, uint32_t height, uint32_t depth)
    : width_(width), height_(height), depth_(depth) {
  for (uint32_t x = 0; x < width_; ++x) {
    for (uint32_t y = 0; y < height_; ++y) {
      for (uint32_t z = 0; z < depth_; ++z) {
        glm::ivec3 chunkPos{static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)};
        chunks_.emplace(chunkPos, std::make_unique<Chunk>(chunkPos));
        dirtyChunks_.emplace(chunkPos);
      }
    }
  }
}

}  // namespace gm