#include "Chunk.h"

#include <cassert>
#include <cstdint>

namespace gm {

Chunk::Chunk(const glm::ivec3& pos) : voxels_(std::make_unique<Voxel[]>(kVolume)), pos_(pos) {
  std::fill(voxels_.get(), voxels_.get() + kVolume, Voxel{});
}

Voxel& Chunk::GetVoxel(const glm::ivec3& localPos) {
  assert(localPos.x >= 0 && localPos.x < kLength);
  assert(localPos.y >= 0 && localPos.y < kLength);
  assert(localPos.z >= 0 && localPos.z < kLength);

  return voxels_[ArrayIndex(localPos)];
}

const Voxel& Chunk::GetVoxel(const glm::ivec3& localPos) const {
  assert(localPos.x >= 0 && localPos.x < kLength);
  assert(localPos.y >= 0 && localPos.y < kLength);
  assert(localPos.z >= 0 && localPos.z < kLength);

  return voxels_[ArrayIndex(localPos)];
}

}  // namespace gm