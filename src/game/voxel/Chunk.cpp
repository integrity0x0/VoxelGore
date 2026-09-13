#include "Chunk.h"

#include <cassert>
#include <cstdint>

namespace gm {

namespace {
uint32_t hashChunkPos(const glm::ivec3& pos) {
  uint32_t h = 2166136261u;
  auto mix = [&h](int32_t v) {
    h ^= static_cast<uint32_t>(v);
    h *= 16777619u;
  };
  mix(pos.x);
  mix(pos.y);
  mix(pos.z);
  return h;
}
}  // namespace

Chunk::Chunk(const glm::ivec3& pos)
    : voxels_(std::make_unique<Voxel[]>(kVolume)), pos_(pos) {
  constexpr int platformHeight = 4;
  constexpr uint32_t oreSpawnChance = 100;

  for (size_t i = 0; i < kVolume; ++i) {
    voxels_[i] = Voxel{0};
  }

  for (int y = 0; y < platformHeight; ++y) {
    for (int x = 0; x < kLength; ++x) {
      for (int z = 0; z < kLength; ++z) {
        size_t index = ArrayIndex({x, y, z});
        voxels_[index] = Voxel{1};
        nonEmptyCount_++;
      }
    }
  }

  uint32_t seed = hashChunkPos(pos_);

  bool spawnsOre = (seed % 100) < oreSpawnChance;
  spawnsOre = true;

  if (spawnsOre) {
    int oreX = static_cast<int>((seed / 100) % kLength);
    int oreZ = static_cast<int>((seed / 10000) % kLength);
    int oreY = platformHeight - 1;

    if (oreX >= 0 && oreX < kLength && oreY >= 0 && oreY < kLength && oreZ >= 0 && oreZ < kLength) {
      size_t oreIndex = ArrayIndex({oreX, oreY, oreZ});
      voxels_[oreIndex] = Voxel{2};
    }
  }
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